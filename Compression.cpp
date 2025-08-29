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

void Compression::MergeRows(std::vector<Block> &OutputStack,
                                          std::vector<Block> &Cr,
                                          std::vector<Block> &BlockStack, //Stack of blocks that have been cut off and are pending
                                          int ParentY) {
                                            std::cout << "Merging rows. Current stack size: " << BlockStack.size() << ", Current row size: " << Cr.size() << std::endl;
  std::vector<Block> CRow = Cr; //Current row of ParentX sized Blocks
  // to hold any leftover cut pieces that need to be appended
  std::vector<Block> PrevLeftovers;
  std::vector<Block> CurrLeftovers;
  bool MergedFlag = false;
  Block EBlock;
  int StackPointer = 0;
  if (BlockStack.size() == 0) {
    BlockStack = Cr;
    return;
  }
  while (StackPointer < BlockStack.size()){
    EBlock = BlockStack[StackPointer];
    MergedFlag = false;
    for (Block NewB  : Cr) {
      // same x range, same label, same z, same ParentY block
      // and C is directly above P
      if (EBlock.XPos == NewB.XPos && EBlock.XSize == NewB.XSize && EBlock.Ch == NewB.Ch &&
          EBlock.ZPos == NewB.ZPos && ((EBlock.YPos / ParentY) == (NewB.YPos / ParentY)) &&
          (NewB.YPos == EBlock.YPos + EBlock.YSize)) 
      {
        // extend vertically
        EBlock.YSize += NewB.YSize;
        // always set YPos to the *lowest index row*
        EBlock.YPos = std::min(EBlock.YPos, NewB.YPos);
        MergedFlag = true;
        break;
      }
    // else if (TryRelaxedMerge(P, C, ParentY, CurrLeftovers, PrevLeftovers)) {
    //          //std::cout << "Relaxed merging block at (" << C.XPos << "," << C.YPos << "," << C.ZPos << ") size (" << C.XSize << "," << C.YSize << "," << C.ZSize << ") with block at (" << P.XPos << "," << P.YPos << "," << P.ZPos << ") size (" << P.XSize << "," << P.YSize << "," << P.ZSize << ")\n";
    //         MergedFlag = true;
    //                              // append any leftover cut pieces
    //          for (auto& l : CurrLeftovers) {
    //              Merged.push_back(l); 
    //          }
    //          // append prev leftovers (these will later be printed in WriteBlocks)
    //          for (auto& l : PrevLeftovers) {
    //              Merged.push_back(l);
    //          }
    //          break;
    //      }
    }
    if (!MergedFlag) {
      std::cout << "StackPointer: " << StackPointer << std::endl;
      std::cout << "Pushing block at (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") " << EBlock.Ch << " to output\n";
      OutputStack.push_back(EBlock);
      BlockStack.erase(BlockStack.begin()+StackPointer);
    } else {StackPointer++;}
    
  }
  for(Block block : Cr) {
    BlockStack.push_back(block);
  }
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
  std::vector<Block> OutputBlocks; // merged blocks for current ParentY group
  std::vector<Block> BlockStack;
  int Height = (int)Rows.size();
  // std::cout << "Processing layer " << LayerNum << " with " << Height << " rows.\n";

  // Iterate bottom -> top
  for (int RowNum = 0; RowNum < Height; RowNum++) {
    std::cout << "  Processing row " << RowNum << " / " << Height - 1 << std::endl;
    int YPos = RowNum; // bottom = 0

    std::vector<Block> CurrRow =
        SingleLineBlocks(Rows[YPos], ParentX, ParentY, ParentZ, YPos, LayerNum);

    // Debug print
    std::cout << "    Current row blocks: ";

    for (Block Block: CurrRow) {
      std::cout <<  Block.XPos << "," << Block.YPos << "," << Block.ZPos << "," << Block.XSize << "," << Block.YSize << "," << Block.ZSize << "," << Block.Ch << " - ";
    }
    std::cout << std::endl;

    MergeRows(OutputBlocks, CurrRow, BlockStack, ParentY);

    // If we've completed a ParentY block or hit the last row, flush
    if ((RowNum + 1) % ParentY == 0 || RowNum == Height - 1) {
      for(Block block : BlockStack) {
        OutputBlocks.push_back(block);
      }
      WriteBlocks(OutputBlocks, Output, TagTable);
      OutputBlocks.clear();
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
