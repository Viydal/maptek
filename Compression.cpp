#include "Compression.h"

Compression::Compression() {}

void Compression::FormatOutput(std::ostringstream &Output, int XPos, int RowNum,
                               int LayerNum, int NumX, int NumY, int NumZ,
                               char Ch,
                               const std::string* TagTable) {
  Output << XPos << "," << RowNum << "," << LayerNum << "," << NumX << ","
         << NumY << "," << NumZ << "," << TagTable[Ch] << "\n";
  return;
}

std::string Compression::SingleLineCompress(
    const std::string Row, std::string* TagTable,
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
                    std::vector<Block>& BlockStack,
                    std::vector<Block>& OutputStack) 
{
    // Rule 0: must be same "row group"
    if ((prev.YPos / ParentY) != (curr.YPos / ParentY)) return false;
    if (prev.ZPos != curr.ZPos) return false;
    if (prev.Ch != curr.Ch) return false;

    // --- compute overlap in x
    int StartMerge = std::max(prev.XPos, curr.XPos);
    int EndMerge   = std::min(prev.XPos + prev.XSize,
                        curr.XPos + curr.XSize);
    int overlap = EndMerge - StartMerge;
    if (overlap <= 0) return false; // no horizontal overlap

    // --- Rule 2: cannot shrink prev overlap too much
    if (overlap <= prev.XSize / 2) {
        return false;
    }

        // --- split prev into (left, overlap, right)
    int prev_left  = StartMerge - prev.XPos;
    int prev_right = (prev.XPos + prev.XSize) - EndMerge;
    // --- split curr into (left, overlap, right)
    int curr_left  = StartMerge - curr.XPos;
    int curr_right = (curr.XPos + curr.XSize) - EndMerge;

    // enforce Rule 1/3/4 by tracking "which side relaxed"
    // Right now we only allow one side trimming for prev and curr
    if (prev_left > 0 && prev_right > 0) return false;
    if (curr_left > 0 && curr_right > 0) return false;
    if ((prev_left > 0 && curr_right > 0) ||
        (prev_right > 0 && curr_left > 0)) return false;

    //std::cout << "\nRelaxed merging block at (" << prev.XPos << "," << prev.YPos << "," << prev.ZPos << ") size (" << prev.XSize << "," << prev.YSize << "," << prev.ZSize << ") with block at (" << curr.XPos << "," << curr.YPos << "," << curr.ZPos << ") size (" << curr.XSize << "," << curr.YSize << "," << curr.ZSize << ")\n";
        // --- leftovers from prev: must be flushed immediately
    if (prev_left > 0) {
        Block left = {prev.XPos, prev.YPos, prev.ZPos,
                      prev_left, prev.YSize, prev.ZSize, prev.Ch};
        OutputStack.push_back(left);
        //std::cout << "Prev left Output: (" << left.XPos << "," << left.YPos << "," << left.ZPos << ") size (" << left.XSize << "," << left.YSize << "," << left.ZSize << ") - " << left.Ch << "\n";
    } else if (prev_right > 0) {
        Block right = {EndMerge, prev.YPos, prev.ZPos,
                       prev_right, prev.YSize, prev.ZSize, prev.Ch};
        OutputStack.push_back(right);
        //std::cout << "Prev right Output: (" << right.XPos << "," << right.YPos << "," << right.ZPos << ") size (" << right.XSize << "," << right.YSize << "," << right.ZSize << ") - " << right.Ch << "\n";
    }

    // --- perform the merge using overlap region
    prev.XPos  = StartMerge;
    prev.XSize = overlap;
    prev.YSize += curr.YSize;

    if (curr_left > 0) {
        Block left = {curr.XPos, curr.YPos, curr.ZPos,
                      curr_left, curr.YSize, curr.ZSize, curr.Ch};
        BlockStack.push_back(left);
        //std::cout << "Curr left Block Stack: (" << left.XPos << "," << left.YPos << "," << left.ZPos << ") size (" << left.XSize << "," << left.YSize << "," << left.ZSize << ") - " << left.Ch << "\n";
    } else if (curr_right > 0) {
        Block right = {EndMerge, curr.YPos, curr.ZPos,
                       curr_right, curr.YSize, curr.ZSize, curr.Ch};
        BlockStack.push_back(right);
        //std::cout << "  Curr right BlockStack: (" << right.XPos << "," << right.YPos << "," << right.ZPos << ") size (" << right.XSize << "," << right.YSize << "," << right.ZSize << ") - " << right.Ch << "\n";
    }


    return true;
}

void Compression::MergeRows(std::vector<Block> &OutputStack,
                                          std::vector<Block> &Cr,
                                          std::vector<Block> &BlockStack, //Stack of blocks that have been cut off and are pending
                                          int ParentY) {
  //std::vector<Block> CRow = Cr; //Current row of ParentX sized Blocks
  // to hold any leftover cut pieces that need to be appended
  bool MergedFlag = false;
  //Block EBlock;
  int StackPointer = 0;
  if (BlockStack.size() == 0) {
    BlockStack = Cr;
    return;
  }

  size_t BsSize = BlockStack.size();
  size_t CrSize = Cr.size();
  while (StackPointer < BsSize){
    //EBlock = BlockStack[StackPointer];
    MergedFlag = false;
    
    for (size_t NewBPos = 0; NewBPos < CrSize; NewBPos++) {
      
      //Block NewB = Cr[NewBPos];
      if (BlockStack[StackPointer].Ch == Cr[NewBPos].Ch){
      // same x range, same label, same z, same ParentY block
      // and C is directly above P
      if (BlockStack[StackPointer].XPos == Cr[NewBPos].XPos && BlockStack[StackPointer].XSize == Cr[NewBPos].XSize &&
          BlockStack[StackPointer].ZPos == Cr[NewBPos].ZPos && ((BlockStack[StackPointer].YPos / ParentY) == (Cr[NewBPos].YPos / ParentY)) &&
          (Cr[NewBPos].YPos == BlockStack[StackPointer].YPos + BlockStack[StackPointer].YSize)) 
      {
        //std::cout << "Perfect Merging at (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";
        // extend vertically
        BlockStack[StackPointer].YSize += Cr[NewBPos].YSize;
        // always set YPos to the *lowest index row*
        BlockStack[StackPointer].YPos = std::min(BlockStack[StackPointer].YPos, Cr[NewBPos].YPos);
        //BlockStack[StackPointer] = BlockStack[StackPointer];
        MergedFlag = true;
        Cr.erase(Cr.begin()+NewBPos);
        CrSize--;
        NewBPos--;
        goto NEXTBLOCK;
      } else if (TryRelaxedMerge(BlockStack[StackPointer], Cr[NewBPos], ParentY, Cr, OutputStack)) {
              //std::cout << "Relaxed merging block at (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";
              //BlockStack[StackPointer] = BlockStack[StackPointer];
              MergedFlag = true;
              Cr.erase(Cr.begin()+NewBPos);
              CrSize--;
              NewBPos--;
              goto NEXTBLOCK;
          } else {
            //std::cout << "No Merging (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";
          }
    } //else {std::cout << "No Merging (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";}
    }
    NEXTBLOCK:
    if (!MergedFlag) {
      //std::cout << "Adding most recent block to Output\n"; 
      OutputStack.push_back(BlockStack[StackPointer]);
      BlockStack.erase(BlockStack.begin()+StackPointer);
      BsSize--;
    } else {StackPointer++;}
    //std::cout <<"SP: " << StackPointer << " StackSize: " << BlockStack.size() << " CR_Size:" << Cr.size() << "\n\n";
      

  }
  for(Block block : Cr) {
    BlockStack.push_back(block);
  }
}

void Compression::WriteBlocks(
    const std::vector<Block> &Blocks, std::ostringstream &Output,
    const std::string* TagTable) {
  for (const auto &B : Blocks) {
    FormatOutput(Output, B.XPos, B.YPos, B.ZPos, B.XSize, B.YSize, B.ZSize,
                 B.Ch, TagTable);
  }
}

void Compression::ProcessLayer(
    const std::vector<std::vector<Block>> &Rows, int ParentX, int ParentY, int ParentZ,
    int LayerNum, std::ostringstream &Output,
    const std::string* TagTable) {
  std::vector<Block> OutputBlocks; // merged blocks for current ParentY group
  std::vector<Block> BlockStack;
  int Height = (int)Rows.size();

  // Iterate bottom -> top
  for (int RowNum = 0; RowNum < Height; RowNum++) {
    int YPos = RowNum; // bottom = 0

    std::vector<Block> CurrRow = Rows[YPos];

    //for (Block Block: CurrRow) {
    //  std::cout <<  Block.XPos << "," << Block.YPos << "," << Block.ZPos << "," << Block.XSize << "," << Block.YSize << "," << Block.ZSize << "," << Block.Ch << " - ";
    //}
    //std::cout << std::endl;

    MergeRows(OutputBlocks, CurrRow, BlockStack, ParentY);

    // If we've completed a ParentY block or hit the last row, flush
    if ((RowNum + 1) % ParentY == 0 || RowNum == Height - 1) {
      //std::cout << "\n Clearing Blockstack and writing output\n\n";
      WriteBlocks(OutputBlocks, Output, TagTable);
      OutputBlocks.clear();
      if (!BlockStack.empty()) {
        WriteBlocks(BlockStack, Output, TagTable);
        BlockStack.clear();
      }
    }
  }
  
}

std::string Compression::FormatOutputStrings(
    std::ostringstream &Output, int XPos, int RowNum, int LayerNum, int NumX,
    int NumY, int NumZ, char Ch,
    const std::string* TagTable) {
  Output << XPos << "," << RowNum << "," << LayerNum << "," << NumX << ","
         << NumY << "," << NumZ << "," << TagTable[Ch] << "\n";
  return Output.str();
}

std::vector<std::string> Compression::WriteBlocksVectorStrings(
    const std::vector<Block> &Blocks, std::ostringstream &Output,
    const std::string* TagTable) {
  std::vector<std::string> Result;
  for (const auto &B : Blocks) {
    Result.push_back(FormatOutputStrings(Output, B.XPos, B.YPos, B.ZPos,
                                         B.XSize, B.YSize, B.ZSize, B.Ch,
                                         TagTable));
  }
  return Result;
}