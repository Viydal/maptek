#include "Compression.h"

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
      // XPosition, YPosition, ZPosition, Xsize, Ysize, Zsize, label
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

bool Compression::TryRelaxedMerge(Block& prev,
                     Block& curr,
                     int ParentY,
                     std::vector<Block>& currLeftovers,
                     std::vector<Block>& prevLeftovers) 
{
    // Rule 0: must be same "row group"
    if ((prev.YPos / ParentY) != (curr.YPos / ParentY)) return false;
    if (prev.ZPos != curr.ZPos) return false;
    if (prev.Ch != curr.Ch) return false;

    // --- compute overlap in x
    int start = std::max(prev.XPos, curr.XPos);
    int end   = std::min(prev.XPos + prev.XSize,
                         curr.XPos + curr.XSize);
    int overlap = end - start;
    if (overlap <= 0) return false; // no horizontal overlap

    // --- Rule 2: cannot shrink prev overlap too much
    if (overlap <= prev.XSize / 2) {
        return false;
    }

        // --- split prev into (left, overlap, right)
    int prev_left  = start - prev.XPos;
    int prev_right = (prev.XPos + prev.XSize) - end;
    // --- split curr into (left, overlap, right)
    int curr_left  = start - curr.XPos;
    int curr_right = (curr.XPos + curr.XSize) - end;

    // enforce Rule 1/3/4 by tracking "which side relaxed"
    // Right now we only allow one side trimming for prev and curr
    if (prev_left > 0 && prev_right > 0) return false;
    if (curr_left > 0 && curr_right > 0) return false;
    if ((prev_left > 0 && curr_right > 0) ||
        (prev_right > 0 && curr_left > 0)) return false;

        // --- leftovers from prev: must be flushed immediately
    if (prev_left > 0) {
        Block left = {prev.XPos, prev.YPos, prev.ZPos,
                      prev_left, prev.YSize, prev.ZSize, prev.Ch};
        prevLeftovers.push_back(left);
        // std::cout << "Prev left leftover: (" << left.XPos << "," << left.YPos << "," << left.ZPos << ") size (" << left.XSize << "," << left.YSize << "," << left.ZSize << ")" << left.Ch << "\n";
    }
    if (prev_right > 0) {
        Block right = {end, prev.YPos, prev.ZPos,
                       prev_right, prev.YSize, prev.ZSize, prev.Ch};
        prevLeftovers.push_back(right);
        // std::cout << "Prev right leftover: (" << right.XPos << "," << right.YPos << "," << right.ZPos << ") size (" << right.XSize << "," << right.YSize << "," << right.ZSize << ")" << right.Ch << "\n";
    }

    // --- perform the merge using overlap region
    prev.XPos  = start;
    prev.XSize = overlap;
    prev.YSize += curr.YSize;

    if (curr_left > 0) {
        Block left = {curr.XPos, curr.YPos, curr.ZPos,
                      curr_left, curr.YSize, curr.ZSize, curr.Ch};
        currLeftovers.push_back(left);
        // std::cout << "Curr left leftover: (" << left.XPos << "," << left.YPos << "," << left.ZPos << ") size (" << left.XSize << "," << left.YSize << "," << left.ZSize << ")" << left.Ch << "\n";
    }
    if (curr_right > 0) {
        Block right = {end, curr.YPos, curr.ZPos,
                       curr_right, curr.YSize, curr.ZSize, curr.Ch};
        currLeftovers.push_back(right);
        // std::cout << "Curr right leftover: (" << right.XPos << "," << right.YPos << "," << right.ZPos << ") size (" << right.XSize << "," << right.YSize << "," << right.ZSize << ")" << right.Ch << "\n";
    }


    return true;
}

std::vector<Block> Compression::MergeRows(const std::vector<Block> &PrevRow,
                                          const std::vector<Block> &CurrRow,
                                          int ParentY) {
  std::vector<Block> Merged = PrevRow;

    // to hold any leftover cut pieces that need to be appended
    std::vector<Block> PrevLeftovers;
    std::vector<Block> CurrLeftovers;

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
      else if (TryRelaxedMerge(P, C, ParentY, CurrLeftovers, PrevLeftovers)) {
                // std::cout << "Relaxed merging block at (" << c.XPos << "," << c.YPos << "," << c.ZPos << ") size (" << c.XSize << "," << c.YSize << "," << c.ZSize << ") with block at (" << p.XPos << "," << p.YPos << "," << p.ZPos << ") size (" << p.XSize << "," << p.YSize << "," << p.ZSize << ")\n";
                MergedFlag = true;
                                    // append any leftover cut pieces
                for (auto& l : CurrLeftovers) {
                    Merged.push_back(l); 
                }
                break;
            }
    }
    if (!MergedFlag) {
      Merged.push_back(C);
    }
  }

      // append prev leftovers (these will later be printed in WriteBlocks)
    for (auto& l : PrevLeftovers) {
        Merged.push_back(l);
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
