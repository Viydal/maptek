#include "Compression.h"
#include <algorithm>

Compression::Compression() {}



bool Compression::TryRelaxedMerge(Block& prev, Block& curr, int ParentY, std::vector<Block>& BlockStack, std::vector<Block>& OutputStack) {
    // Rule 0: must be same "row group"
    if ((prev.YPos / ParentY) != (curr.YPos / ParentY)) return false;
    if (prev.ZPos != curr.ZPos) return false;
    if (prev.Ch != curr.Ch) return false;

    // --- compute overlap in x
    int StartMerge = std::max(prev.XPos, curr.XPos);
    int EndMerge   = std::min(prev.XPos + prev.XSize, curr.XPos + curr.XSize);
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
    if ((prev_left > 0 && curr_right > 0) || (prev_right > 0 && curr_left > 0)) return false;

    //std::cout << "\nRelaxed merging block at (" << prev.XPos << "," << prev.YPos << "," << prev.ZPos << ") size (" << prev.XSize << "," << prev.YSize << "," << prev.ZSize << ") with block at (" << curr.XPos << "," << curr.YPos << "," << curr.ZPos << ") size (" << curr.XSize << "," << curr.YSize << "," << curr.ZSize << ")\n";
    // --- leftovers from prev: must be flushed immediately
    if (prev_left > 0) {
        Block left = {prev.XPos, prev.YPos, prev.ZPos, prev_left, prev.YSize, prev.ZSize, prev.Ch};
        OutputStack.push_back(left);
        //std::cout << "Prev left Output: (" << left.XPos << "," << left.YPos << "," << left.ZPos << ") size (" << left.XSize << "," << left.YSize << "," << left.ZSize << ") - " << left.Ch << "\n";
    } else if (prev_right > 0) {
        Block right = {EndMerge, prev.YPos, prev.ZPos, prev_right, prev.YSize, prev.ZSize, prev.Ch};
        OutputStack.push_back(right);
        //std::cout << "Prev right Output: (" << right.XPos << "," << right.YPos << "," << right.ZPos << ") size (" << right.XSize << "," << right.YSize << "," << right.ZSize << ") - " << right.Ch << "\n";
    }

    // --- perform the merge using overlap region
    prev.XPos  = StartMerge;
    prev.XSize = overlap;
    prev.YSize += curr.YSize;

    if (curr_left > 0) {
        Block left = {curr.XPos, curr.YPos, curr.ZPos, curr_left, curr.YSize, curr.ZSize, curr.Ch};
        BlockStack.push_back(left);
        //std::cout << "Curr left Block Stack: (" << left.XPos << "," << left.YPos << "," << left.ZPos << ") size (" << left.XSize << "," << left.YSize << "," << left.ZSize << ") - " << left.Ch << "\n";
    } else if (curr_right > 0) {
        Block right = {EndMerge, curr.YPos, curr.ZPos, curr_right, curr.YSize, curr.ZSize, curr.Ch};
        BlockStack.push_back(right);
        //std::cout << "  Curr right BlockStack: (" << right.XPos << "," << right.YPos << "," << right.ZPos << ") size (" << right.XSize << "," << right.YSize << "," << right.ZSize << ") - " << right.Ch << "\n";
    }

    return true;
}

void Compression::MergeRows(std::vector<Block> &OutputStack, std::vector<Block> &Cr, std::vector<Block> &BlockStack, int ParentY) {
  bool MergedFlag = false;
  Block EBlock;
  int StackPointer = 0;

  // If BlockStack is empty, just load current row
  if (BlockStack.size() == 0) {
    BlockStack = Cr;
    return;
  }

  // Iterate existing stack blocks
  while (StackPointer < BlockStack.size()){
    EBlock = BlockStack[StackPointer];
    MergedFlag = false;
    
    // Compare against each new row block
    for (size_t NewBPos = 0; NewBPos < Cr.size(); NewBPos++) {
      
      Block NewB = Cr[NewBPos];
      if (EBlock.Ch == NewB.Ch){
      // Case 1: Perfect merge
      if (EBlock.XPos == NewB.XPos && EBlock.XSize == NewB.XSize &&
          EBlock.ZPos == NewB.ZPos && ((EBlock.YPos / ParentY) == (NewB.YPos / ParentY)) &&
          (NewB.YPos == EBlock.YPos + EBlock.YSize)) 
      {
        //std::cout << "Perfect Merging at (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";
        // extend vertically
        EBlock.YSize += NewB.YSize;
        // always set YPos to the *lowest index row*    
        EBlock.YPos = std::min(EBlock.YPos, NewB.YPos);
        BlockStack[StackPointer] = EBlock;
        MergedFlag = true;
        Cr.erase(Cr.begin()+NewBPos);
        NewBPos--;
        goto NEXTBLOCK;

        // Case 2: Relaxed merge
      } else if (TryRelaxedMerge(EBlock, NewB, ParentY, Cr, OutputStack)) {
              //std::cout << "Relaxed merging block at (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";
              BlockStack[StackPointer] = EBlock;
              MergedFlag = true;
              Cr.erase(Cr.begin()+NewBPos);
              NewBPos--;
              goto NEXTBLOCK;
          } else {
            //std::cout << "No Merging (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";
          }
    } //else {std::cout << "No Merging (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";}
    }
    NEXTBLOCK:
    if (!MergedFlag) {
      // Couldn’t merge → flush to output
      //std::cout << "Adding most recent block to Output\n"; 
      OutputStack.push_back(EBlock);
      BlockStack.erase(BlockStack.begin()+StackPointer);
    } else {StackPointer++;}
    //std::cout <<"SP: " << StackPointer << " StackSize: " << BlockStack.size() << " CR_Size:" << Cr.size() << "\n\n";
      

  }
  // Push remaining new row blocks onto stack
  for(Block block : Cr) {
    BlockStack.push_back(block);
  }
}
void Compression::FormatOutput(std::ostringstream &Output, int XPos, int RowNum, int LayerNum, int NumX, int NumY, int NumZ, char Ch, const std::string* TagTable) {
  Output << XPos << "," << RowNum << "," << LayerNum << "," << NumX << "," << NumY << "," << NumZ << "," << TagTable[Ch] << "\n";
  return;
}

void Compression::WriteBlocks(std::vector<Block>& Blocks, std::ostringstream &Output, const std::string* TagTable) {
  FormatSubmit(Blocks);
  for (const auto &B : Blocks) {
    FormatOutput(Output, B.XPos, B.YPos, B.ZPos, B.XSize, B.YSize, B.ZSize, B.Ch, TagTable);
  }
}

void Compression::WriteBlocksString(std::vector<Block>& Blocks, std::ostringstream &Output, const std::string* TagTable) {
  FormatSubmit(Blocks);
  
  std::string out;
  Block B;
  for (int i = 0; i < Blocks.size(); i++) { 
    B = Blocks[i];
    out += std::to_string(B.XPos);
    out += ',';
    out += std::to_string(B.YPos);
    out += ',';
    out += std::to_string(B.ZPos);
    out += ',';
    out += std::to_string(B.XSize);
    out += ',';
    out += std::to_string(B.YSize);
    out += ',';
    out += std::to_string(B.ZSize);
    out += ',';
    out += TagTable[B.Ch];
    out += '\n';
    //FormatOutput(Output, B.XPos, B.YPos, B.ZPos, B.XSize, B.YSize, B.ZSize, B.Ch, TagTable);
  }
  std::cout<<out;
}

void Compression::FormatSubmit(std::vector<Block> &OutputBlocks) {
  std::sort(OutputBlocks.begin(), OutputBlocks.end(), [](const Block& a, const Block& b) {
    if (a.ZPos != b.ZPos) return a.ZPos < b.ZPos;  // Sort Z axis (first)
    if (a.YPos != b.YPos) return a.YPos < b.YPos;  // Sort Y axis (second)
    return a.XPos < b.XPos;  // Sort X axis (third)
  });
}

void Compression::MergeLayers(std::vector<Block>& Blocks, int ParentZ) {
  if (Blocks.empty()) return;
  
  // Sort all blocks by position (X, Y, then Z)
  std::sort(Blocks.begin(), Blocks.end(), [](const Block& a, const Block& b) {
    if (a.XPos != b.XPos) return a.XPos < b.XPos;
    if (a.YPos != b.YPos) return a.YPos < b.YPos;
    if (a.Ch != b.Ch) return a.Ch < b.Ch;
    return a.ZPos < b.ZPos;
  });

  FinalBlocks.clear();
  FinalBlocks.reserve(Blocks.size());
  
  for (size_t i = 0; i < Blocks.size(); i++) {
    Block Current = Blocks[i];
    
    // Try to merge with blocks that have same X, Y, Size, and Ch
    while (i + 1 < Blocks.size()) {
      const Block& Next = Blocks[i + 1];
      
      // Can the blocks be merged?
      bool canMerge = (Current.XPos == Next.XPos && 
                      Current.YPos == Next.YPos &&
                      Current.XSize == Next.XSize && 
                      Current.YSize == Next.YSize &&
                      Current.Ch == Next.Ch &&
                      Current.ZPos + Current.ZSize == Next.ZPos);
      
      if (!canMerge) break;

      int TotalZSize = Current.ZSize + Next.ZSize;
      int MergedEndZ = (Current.ZPos % ParentZ) + TotalZSize;

      // Check if merged size fits within parent
      if (MergedEndZ <= ParentZ) {
        // Merge blocks
        Current.ZSize = TotalZSize;
        i++; // Skip the merged block
      } else {
        break; // Can't merge without exceeding parent size
      }
    }
    FinalBlocks.push_back(Current);
  }
}


bool Compression::TryRelaxedLayerMerge(Block& Current, Block& Next, int ParentZ, std::vector<Block>& LeftOvers) {
  // Rule 0: two Blocks must be in the same parent block segment
  if ((Current.ZPos / ParentZ) != (Next.ZPos / ParentZ)) return false;

  // general check to ensure the Blocks are eligible for merging
  if ((Current.ZPos % ParentZ) + (Current.ZSize + Next.ZSize) > ParentZ) return false;
  if (Current.ZPos + Current.ZSize != Next.ZPos) return false;
  if (Current.YPos != Next.YPos) return false;
  if (Current.YSize != Next.YSize) return false;
  if (Current.Ch != Next.Ch) return false;

  // --- compute overlap in z layers based off their x coordinate
  int StartMerge = std::max(Current.XPos, Next.XPos);
  int EndMerge = std::min(Current.XPos + Current.XSize, Next.XPos + Next.XSize);
  int overlap = EndMerge - StartMerge;
  if (overlap <= 0) return false; // no horizontal overlap

  // --- Rule 2: cannot shrink current block overlap too much
  if (overlap <= Current.XSize / 2) {
      return false;
  }

  // --- split current into (Left, overlap, Right)
  int CurrentLeft = StartMerge - Current.XPos;
  int CurrentRight = (Current.XPos + Current.XSize) - EndMerge;
  // --- split next into (Left, overlap, Right)
  int NextLeft = StartMerge - Next.XPos;
  int NextRight = (Next.XPos + Next.XSize) - EndMerge;

  // enforce Rule 1/3/4 by tracking "which side relaxed"
  // Right now we only allow one side trimming for prev and curr
  if (CurrentLeft > 0 && CurrentRight > 0) return false;
  if (NextLeft > 0 && NextRight > 0) return false;
  if ((CurrentLeft > 0 && NextRight > 0) || (CurrentRight > 0 && NextLeft > 0)) return false;

  // making sure that relaxed z compression doesnt create more blocks
  int CountBefore = 2;
  int CountAfter = 1;
  if (CurrentLeft > 0) CountAfter++;
  if (CurrentRight > 0) CountAfter++;
  if (NextLeft > 0) CountAfter++;
  if (NextRight > 0) CountAfter++;

  if (CountAfter >= CountBefore) return false;

  // ALL TEST CASES PASSED - COMMENCE RELAXED MERGE

  // --- LeftOvers from merge: must be flushed immediately
  if (CurrentLeft > 0) {
      Block Left = {Current.XPos, Current.YPos, Current.ZPos, CurrentLeft, Current.YSize, Current.ZSize, Current.Ch};
      LeftOvers.push_back(Left);
  } else if (CurrentRight > 0) {
      Block Right = {EndMerge, Current.YPos, Current.ZPos, CurrentRight, Current.YSize, Current.ZSize, Current.Ch};
      LeftOvers.push_back(Right);
  }

  // --- perform the merge using overlap region
  Current.XPos = StartMerge;
  Current.XSize = overlap;
  Current.ZSize += Next.ZSize;

  if (NextLeft > 0) {
      Block Left = {Next.XPos, Next.YPos, Next.ZPos, NextLeft, Next.YSize, Next.ZSize, Next.Ch};
      LeftOvers.push_back(Left);
  } else if (NextRight > 0) {
      Block Right = {EndMerge, Next.YPos, Next.ZPos, NextRight, Next.YSize, Next.ZSize, Next.Ch};
      LeftOvers.push_back(Right);
  }

  return true;
}


void Compression::ProcessLayer(const std::vector<std::vector<Block>> &Rows, int ParentX, int ParentY, int ParentZ, int LayerNum, std::ostringstream &Output, const std::string* TagTable) {
  std::vector<Block> OutputBlocks; // merged blocks for Current ParentY group
  std::vector<Block> BlockStack;
  int Height = (int)Rows.size();

  // Iterate bottom -> top
  for (int RowNum = 0; RowNum < Height; RowNum++) {
    int YPos = RowNum; // bottom = 0
    std::vector<Block> CurrRow = Rows[YPos];
    //std::cout<<"Merging Rows... \n";
    MergeRows(OutputBlocks, CurrRow, BlockStack, ParentY);
    
    // If we've completed a ParentY block or hit the last row, flush
    if ((RowNum + 1) % ParentY == 0 || RowNum == Height - 1) {
      //std::cout << "\n Clearing Blockstack and writing output\n\n";
      AllLayerBlocks.insert(AllLayerBlocks.end(), OutputBlocks.begin(), OutputBlocks.end());
      AllLayerBlocks.insert(AllLayerBlocks.end(), BlockStack.begin(), BlockStack.end());

      OutputBlocks.clear();
      BlockStack.clear();
    }
  }
}

std::vector<Block>& Compression::GetBlocks(){
  return AllLayerBlocks;
}

size_t Compression::GetBlocksSize(){
  return AllLayerBlocks.size();
}

std::vector<Block>& Compression::GetFinalBlocks(){
  return FinalBlocks;
}