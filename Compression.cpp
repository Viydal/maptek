#include "Compression.h"
#include <cmath>
Compression::Compression() {}

void Compression::FormatOutput(std::ostringstream &Output, int XPos, int RowNum,
                               int LayerNum, int NumX, int NumY, int NumZ,
                               char Ch,
                               std::unordered_map<char, std::string> TagTable) {
  Output << XPos << "," << RowNum << "," << LayerNum << "," << NumX << ","
         << NumY << "," << NumZ << "," << TagTable[Ch] << "\n";
  return;
}

std::string Compression::SingleLineCompress(
    const std::string Row, std::unordered_map<char, std::string> TagTable,
    int ParentX, int ParentY, int ParentZ, int RowNum, int LayerNum) {
  size_t Count = 0;
  int XPos = 0; // track where each run starts
  std::ostringstream Output;

  // parse through the string
  while (Count < Row.size()) {
    // read number
    int Num = 0;
    while (Count < Row.size() && isdigit(Row[Count])) {
      Num = Num * 10 + (Row[Count] - '0');
      Count++;
    }

    // read character (label)
    char Ch = Row[Count];
    Count++;

    // break the run into ParentX-sized chunks
    while (Num > 0) {
      int Remaining = ParentX - (XPos % ParentX); // space left in this block
      int Chunk = std::min(Num, Remaining);
      // append line in the format:
      // x_position, y_position, z_position, Xsize, Ysize, Zsize, label
      FormatOutput(Output, XPos, RowNum, LayerNum, Chunk, 1, 1, Ch, TagTable);
      // advance x position
      XPos += Chunk;
      Num -= Chunk;
    }
  }

  return Output.str();
}

std::vector<Block> Compression::SingleLineBlocks(const std::string Row,
                                                 int ParentX, int ParentY,
                                                 int ParentZ, int RowNum,
                                                 int LayerNum) {
  size_t Count = 0;
  int XPos = 0;
  std::vector<Block> Blocks;

  while (Count < Row.size()) {
    // read number
    int Num = 0;
    while (Count < Row.size() && isdigit(Row[Count])) {
      Num = Num * 10 + (Row[Count] - '0');
      Count++;
    }

    char Ch = Row[Count];
    Count++;

    while (Num > 0) {
      int Remaining = ParentX - (XPos % ParentX);
      int Chunk = std::min(Num, Remaining);

      Blocks.push_back({XPos, RowNum, LayerNum, Chunk, 1, 1, Ch});

      XPos += Chunk;
      Num -= Chunk;
    }
  }
  return Blocks;
}

std::vector<Block> Compression::MergeRows(const std::vector<Block> &PrevRow,
                                          const std::vector<Block> &CurrRow,
                                          int ParentY) {
  std::vector<Block> Merged = PrevRow;

  for (auto &C : CurrRow) {
    bool MergedFlag = false;
    for (auto &P : Merged) {
      // same x range, same label, same z, same ParentY block
      // and C is directly above P
      if (P.XPos == C.XPos && P.XSize == C.XSize && P.Ch == C.Ch &&
          P.ZPos == C.ZPos && ((P.YPos / ParentY) == (C.YPos / ParentY)) &&
          (C.YPos == P.YPos + P.YSize)) {

        // extend vertically
        P.YSize += C.YSize;

        // always set YPos to the *lowest row*
        P.YPos = std::min(P.YPos, C.YPos);

        MergedFlag = true;
        break;
      }
    }
    if (!MergedFlag) {
      Merged.push_back(C);
    }
  }

  return Merged;
}

void Compression::WriteBlocks(
    const std::vector<Block> &Blocks, std::ostringstream &Output,
    const std::unordered_map<char, std::string> &TagTable) {
  for (const auto &B : Blocks) {
    FormatOutput(Output, B.XPos, B.YPos, B.ZPos, B.XSize, B.YSize, B.ZSize,
                 B.Ch, TagTable);
  }
}

void Compression::ProcessLayer(
    const std::vector<std::string> &Rows, int ParentX, int ParentY, int ParentZ,
    int LayerNum, std::ostringstream &Output,
    const std::unordered_map<char, std::string> &TagTable) {
  std::vector<Block> Accumulated; // merged blocks for current ParentY group

  int Height = (int)Rows.size();

  // Iterate bottom -> top
  for (int RowNum = 0; RowNum < Height; RowNum++) {
    int YPos = RowNum; // bottom = 0

    auto CurrRow =
        SingleLineBlocks(Rows[YPos], ParentX, ParentY, ParentZ, YPos, LayerNum);

    Accumulated = MergeRows(Accumulated, CurrRow, ParentY);

    // If we've completed a ParentY block or hit the last row, flush
    if ((RowNum + 1) % ParentY == 0 || RowNum == Height - 1) {
      WriteBlocks(Accumulated, Output, TagTable);
      Accumulated.clear();
    }
  }
}

std::string Compression::FormatOutputStrings(
    std::ostringstream &Output, int XPos, int RowNum, int LayerNum, int NumX,
    int NumY, int NumZ, char Ch,
    std::unordered_map<char, std::string> TagTable) {
  Output << XPos << "," << RowNum << "," << LayerNum << "," << NumX << ","
         << NumY << "," << NumZ << "," << TagTable[Ch] << "\n";
  return Output.str();
}

std::vector<std::string> Compression::WriteBlocksVectorStrings(
    const std::vector<Block> &Blocks, std::ostringstream &Output,
    const std::unordered_map<char, std::string> &TagTable) {
  std::vector<std::string> Result;
  for (const auto &B : Blocks) {
    Result.push_back(FormatOutputStrings(Output, B.XPos, B.YPos, B.ZPos,
                                         B.XSize, B.YSize, B.ZSize, B.Ch,
                                         TagTable));
  }
  return Result;
}
