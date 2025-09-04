#include "Compression.h"
#include <algorithm>

Compression::Compression() {}

void Compression::FormatOutput(std::ostringstream &Output, int XPos, int RowNum, int LayerNum, int NumX, int NumY, int NumZ, char Ch, const std::string* TagTable) {
  Output << XPos << "," << RowNum << "," << LayerNum << "," << NumX << "," << NumY << "," << NumZ << "," << TagTable[Ch] << "\n";
  return;
}

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
  if (BlockStack.size() == 0) {
    //std::cout<<"BlockStack size == 0 \n";
    BlockStack = Cr;
    return;
  }
  while (StackPointer < BlockStack.size()){
    EBlock = BlockStack[StackPointer];
    MergedFlag = false;
    
    for (size_t NewBPos = 0; NewBPos < Cr.size(); NewBPos++) {
      
      Block NewB = Cr[NewBPos];
      if (EBlock.Ch == NewB.Ch){
      ///same x range, same label, same z, same ParentY block
      // and C is directly above P
      if (EBlock.XPos == NewB.XPos && EBlock.XSize == NewB.XSize &&
          EBlock.ZPos == NewB.ZPos &&
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
    } else {//std::cout << "No Merging (" << EBlock.XPos << "," << EBlock.YPos << "," << EBlock.ZPos << ") size (" << EBlock.XSize << "," << EBlock.YSize << "," << EBlock.ZSize << ") with block at (" << NewB.XPos << "," << NewB.YPos << "," << NewB.ZPos << ") size (" << NewB.XSize << "," << NewB.YSize << "," << NewB.ZSize << ")\n";
    }
    }
    NEXTBLOCK:
    if (!MergedFlag) {
      //std::cout << "Adding most recent block to Output\n"; 
      OutputStack.push_back(EBlock);
      BlockStack.erase(BlockStack.begin()+StackPointer);
    } else {StackPointer++;}
    //std::cout <<"SP: " << StackPointer << " StackSize: " << BlockStack.size() << " CR_Size:" << Cr.size() << "\n\n";
      

  }
  for(Block block : Cr) {
    BlockStack.push_back(block);
  }
}

void Compression::WriteBlocks(std::vector<Block>& Blocks, std::ostringstream &Output, const std::string* TagTable) {
  //FormatSubmit(Blocks);
  for (const auto &B : Blocks) {
    FormatOutput(Output, B.XPos, B.YPos, B.ZPos, B.XSize, B.YSize, B.ZSize, B.Ch, TagTable);
  }
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
  
  // Sort all Blocks by position (X, Y, then Z)
  std::sort(Blocks.begin(), Blocks.end(), [](const Block& a, const Block& b) {
    if (a.Ch != b.Ch) return a.Ch < b.Ch;
    if (a.XPos != b.XPos) return a.XPos < b.XPos;
    if (a.YPos != b.YPos) return a.YPos < b.YPos;
    return a.ZPos < b.ZPos;
  });
  
  FinalBlocks.clear();
  FinalBlocks.reserve(Blocks.size());
  std::vector<bool> used(Blocks.size(), false);
  std::vector<Block> LeftOvers;
  
  for (size_t i = 0; i < Blocks.size(); i++) {
    if (used[i]) continue;
      Block Current = Blocks[i];

      // std::cout << i << std::endl;
      
      // Try to merge with Blocks that have same X, Y, Size, and Ch
    for (size_t j = i + 1; j < Blocks.size(); j++) {
      if (used[j]) continue;
      Block& Next = Blocks[j];
      // Early catches
      if (Current.Ch != Next.Ch) break;
      if (Next.ZPos < Current.ZPos + Current.ZSize) break;
      
      // Can the Blocks be merged?
      bool canMerge = (Current.XPos == Next.XPos && 
                      Current.YPos == Next.YPos &&
                      Current.XSize == Next.XSize && 
                      Current.YSize == Next.YSize &&
                      Current.Ch == Next.Ch &&
                      Current.ZPos + Current.ZSize == Next.ZPos);

      if (Current.ZPos + Current.ZSize == Next.ZPos){
        if (canMerge) {
          int TotalZSize = Current.ZSize + Next.ZSize;
          int MergedEndZ = (Current.ZPos % ParentZ) + TotalZSize;

          // Check if merged size fits within parent
          if (MergedEndZ <= ParentZ) {
            // Merge Blocks
            Current.ZSize = TotalZSize;
            used[j] = true;
          } else {
            break; // Can't merge without exceeding parent size
          }
        } else { 
          // LeftOvers.clear();
          // if (TryRelaxedLayerMerge(Current, Next, ParentZ, LeftOvers)) {
          //   // std::cout << "change made" << std::endl;
          //   Blocks[i] = Current;
          //   used[j] = true;
          //   AllLayerBlocks.insert(AllLayerBlocks.end(), LeftOvers.begin(), LeftOvers.end());
          // } else {
          //   break; // Can't merge, move to next Current block
          // }
          break;
        }
      } else {
        // break;
      }
    }
    FinalBlocks.push_back(Current);
  }
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