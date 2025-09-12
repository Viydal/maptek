#include "Compression.h"
#include <algorithm>

Compression::Compression() {}

void Compression::CompressParentBlock(ParentBlock &ParentBlock) {
	ProcessLayerSort(ParentBlock.Blocks, ParentBlock.LimitX, ParentBlock.LimitY, ParentBlock.LimitZ);
	MergeLayers(ParentBlock.Blocks, ParentBlock.LimitZ);
	//printf("ParentBlock Start: %d %d %d\n", ParentBlock.StartX,ParentBlock.StartY, ParentBlock.StartZ);
	RelaxedXY(ParentBlock.Blocks);
	std::sort(ParentBlock.Blocks.begin(), ParentBlock.Blocks.end(),[](const Block& a, const Block& b) {
		if (a.XPos  != b.XPos)  return a.XPos  < b.XPos;
		if (a.YPos  != b.YPos)  return a.YPos  < b.YPos;
		if (a.ZPos  != b.ZPos)  return a.ZPos  < b.ZPos;
		return a.YPos < b.YPos;
	});
	RelaxedZ(ParentBlock.Blocks);

}


void Compression::RelaxedXY(std::vector<Block> &Blocks) {
	std::sort(Blocks.begin(), Blocks.end(),[](const Block& a, const Block& b) {
		if (a.ZPos  != b.ZPos)  return a.ZPos  < b.ZPos;
		if (a.YPos  != b.YPos)  return a.YPos  < b.YPos;
		if (a.XPos  != b.XPos)  return a.XPos  < b.XPos;
		return a.YPos < b.YPos;
	});
	size_t Size = Blocks.size();
	std::vector<int> RecheckI;
	for (size_t i = 0; i < Size; i++) {
		Block &Current = Blocks[i];
		if (Current.Merged) continue;
		//std::cout << "I: " << i << std::endl;
		while(i+1 < Size) {
			Block &Next = Blocks[i+1];
			if (Current.Ch != Next.Ch) {i++; continue;}
			if (Next.Merged) {i++; continue;}
			if (Current.ZPos != Next.ZPos || Current.ZSize != Next.ZSize) {break;}
			//std::cout << "Inner i:" << i << std::endl;
			//std::cout << "Current: " << Current.XPos << " " << Current.YPos << " " << Current.ZPos << " " << Current.XSize << " " << Current.YSize << " " << Current.ZSize<< " " << Current.Ch << "\n";
			//std::cout << "Next:    " << Next.XPos << " " << Next.YPos << " " << Next.ZPos << " " << Next.XSize << " " << Next.YSize << " " << Next.ZSize << " " << Next.Ch << std::endl;
			int startMerge = std::max(Current.XPos, Next.XPos);
			int EndMerge = std::min(Current.XPos+Current.XSize, Next.XPos+Next.XSize);
			int overlap = EndMerge - startMerge;
			if (overlap <= 0) {i++; break;}
			bool RuleOneX = overlap >= Current.XSize / 2;
			if (startMerge == Current.XPos && startMerge == Next.XPos) {
				//std::cout << "One of ends is being reduced" << std::endl;
				if (Next.XSize == overlap){
					//std::cout << "Current block needs relaxing" << std::endl;
					Block NewBlock(EndMerge, Current.YPos, Current.ZPos, Current.XSize - overlap, Current.YSize, Current.ZSize, Current.Ch);
					Current.YSize += Next.YSize;
					Current.XSize = EndMerge;
					Next.Merged = true;
					Blocks.push_back(NewBlock);
					i+= 2;
					continue;
				} else {
					//std::cout << "Next Block Needs relaxing" << std::endl;
					//std::cout << "Inserting at I: " << i+1 << " Size: " << Size << " Block: " << EndMerge << " " << Next.YPos << " " << Next.ZPos << " " << Next.XSize - overlap << " " <<  Next.YSize << " " << Next.ZSize << " " << Next.Ch << std::endl;
					//Blocks.push_back({EndMerge, Next.YPos, Next.ZPos, Next.XSize - overlap, Next.YSize, Next.ZSize, Next.Ch});
					Block NewBlock(EndMerge, Next.YPos, Next.ZPos, Next.XSize - overlap, Next.YSize, Next.ZSize, Next.Ch);
					RecheckI.push_back(i+1);
					Current.YSize += Next.YSize;
					Next.Merged = true;

					Blocks.push_back(NewBlock);
					i++;
					//Size++;
					//Blocks.insert(Blocks.begin()+i+1, 1, NewBlock);
					//i+=3;
				}
				
			} else if (EndMerge == Current.XPos + Current.XSize && EndMerge == Next.XPos + Next.XSize) {
				//std::cout << "One of beginnings is reduced" << std::endl;
				if (Next.XSize == overlap){
					//std::cout << "TESTABC Current block needs relaxing" << std::endl;
					//std::cout << startMerge << " " << overlap << " Block: " << Current.XPos << " " << Current.YPos << " " << Current.ZPos << " " << Current.XSize - overlap << " " <<  Current.YSize << " " << Current.ZSize << " " << Next.Ch << std::endl;
					Block NewBlock(Current.XPos, Current.YPos, Current.ZPos, Current.XSize - overlap, Current.YSize, Current.ZSize, Current.Ch);
					Current.YSize += Next.YSize;
					//Current.YPos = std::min(current)
					Current.XPos = startMerge;
					Current.XSize = overlap;
					Blocks.push_back(NewBlock);
					Blocks[i+1].Merged = true;
					i += 2;
					continue;
				} else {
					//std::cout << "Next Block Needs relaxing" << std::endl;
					//std::cout << "Inserting at I: " << i+1 << " Size: " << Size << " Block: " << Next.XPos << " " << Next.YPos << " " << Next.ZPos << " " << Next.XSize - overlap << " " <<  Next.YSize << " " << Next.ZSize << " " << Next.Ch << std::endl;
					Block NewBlock(Next.XPos, Next.YPos, Next.ZPos, Next.XSize - overlap, Next.YSize, Next.ZSize, Next.Ch);
					RecheckI.push_back(i+1);
					Current.YSize += Next.YSize;
					Next.Merged = true;
					
					Blocks.push_back(NewBlock);
					i++;

					//Size++;
					//Blocks.insert(Blocks.begin()+i+1, 1, NewBlock);
					//i+=3;
				}
			} else {break;}
		}
		if(!RecheckI.empty()){
			i = RecheckI.back()-1;
			RecheckI.pop_back();
		}
	}
};

void Compression::RelaxedZ(std::vector<Block> &Blocks) {
	// std::sort(Blocks.begin(), Blocks.end(),[](const Block& a, const Block& b) {
	// 	if (a.XPos  != b.XPos)  return a.XPos  < b.XPos;
	// 	if (a.YPos  != b.YPos)  return a.YPos  < b.YPos;
	// 	if (a.ZPos  != b.ZPos)  return a.ZPos  < b.ZPos;
	// 	return a.YPos < b.YPos;
	// });
}

/**
 * TryRelaxedMerge
 * ---------------
 * Attempts to merge two blocks (prev and curr) that overlap horizontally
 * but are not perfectly aligned. Allows "relaxed merging" if:
 *  - Both blocks share the same character, Z, and ParentY group
 *  - They overlap by at least half of prev's width
 *  - Only one "side trimming" occurs (no double trims)
 * 
 * Leftover pieces are pushed either to OutputStack (prev leftovers)
 * or BlockStack (curr leftovers).
 */

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

/**
 * MergeRows
 * ---------
 * Attempts to merge blocks across rows (vertical merging).
 * 
 * Rules:
 *  1. Perfect merge: same X range, same label, same Z, directly stacked
 *  2. Relaxed merge: allow partial X overlap with TryRelaxedMerge
 * 
 * Blocks that cannot merge are flushed to OutputStack.
 */

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

/**
 * MergeLayers
 * -----------
 * Attempts to merge blocks vertically along the Z axis.
 * Blocks can merge if they:
 *   - Have identical X, Y, size, and Ch
 *   - Are adjacent in Z
 *   - Do not exceed ParentZ boundaries
 */
void Compression::MergeLayers(std::vector<Block>& Blocks, int ParentZ) {
	
	// Sort all blocks by position (X, Y, then Z)
	std::sort(Blocks.begin(), Blocks.end(), [](const Block& a, const Block& b) {
		if (a.XPos != b.XPos) return a.XPos < b.XPos;
		if (a.YPos != b.YPos) return a.YPos < b.YPos;
		return a.ZPos < b.ZPos;
	});
	
	for (size_t i = 0; i < Blocks.size(); i++) {
		Block& Current = Blocks[i];
		if (Current.Merged) continue;
		// Try to merge with blocks that have same X, Y, Size, and Ch
		while (i + 1 < Blocks.size()) {
			const Block Next = Blocks[i + 1];
			if (Next.Merged) {i++;continue;};
			// Can the blocks be merged?
			bool canMerge = (Current.XPos == Next.XPos && 
											Current.YPos == Next.YPos &&
											Current.XSize == Next.XSize && 
											Current.YSize == Next.YSize &&
											Current.Ch == Next.Ch &&
											Current.ZPos + Current.ZSize == Next.ZPos);
			
			if (!canMerge) break;
			// int TotalZSize = Current.ZSize + Next.ZSize;
			Current.ZSize += Next.ZSize;
			////Blocks.erase(Blocks.begin()+i+1);
			Blocks[i+1].Merged = true;
			i++;
		}
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

/**
 * ProcessLayer
 * ------------
 * Processes a full 2D layer (vector of rows).
 * Merges rows into vertical ParentY-aligned blocks,
 * then flushes completed groups into AllLayerBlocks.
 */

void Compression::ProcessLayer(std::vector<std::vector<Block>> &Rows, int ParentX, int ParentY, int ParentZ, int LayerNum, std::ostringstream &Output, const std::string* TagTable) {
	std::vector<Block> OutputBlocks; // merged blocks for Current ParentY group
	std::vector<Block> BlockStack;
	int Height = (int)Rows.size();
	// Iterate bottom -> top
	for (int RowNum = 0; RowNum < Height; RowNum++) {
		int YPos = RowNum; // bottom = 0
		std::vector<Block> CurrRow = Rows[YPos];

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

void Compression::ProcessLayerSort(std::vector<Block> &Blocks, int ParentX, int ParentY, int ParentZ) {

		std::sort(Blocks.begin(), Blocks.end(),[](const Block& a, const Block& b) {
			if (a.XPos  != b.XPos)  return a.XPos  < b.XPos;
			if (a.ZPos  != b.ZPos)  return a.ZPos  < b.ZPos;
			return a.YPos < b.YPos;
		});
		
		for (size_t i = 0; i < Blocks.size(); i++) {
				Block& Current = Blocks[i];
				while (i + 1 < Blocks.size()) {
					const Block Next = Blocks[i + 1];
					//std::cout << "testinA " << Next.YPos << " " << ParentY << std::endl;
					//std::cout << "After Testing" << (Next.YPos / ParentY) << std::endl;
					bool canMerge = Current.Ch == Next.Ch && Current.XPos == Next.XPos && 
					Current.XSize == Next.XSize && Current.ZPos == Next.ZPos && 
					(Next.YPos == Current.YPos + Current.YSize);
					//std::cout << "After" << std::endl;
					if (!canMerge) break;

					int TotalYSize = Current.YSize + Next.YSize;
					Current.YSize = TotalYSize;
					Blocks[i+1].Merged = true;
					i++;
					
				}
		}
}


/**
 * GetBlocks
 * ---------
 * Returns all intermediate blocks accumulated across layers.
 */
std::vector<Block>& Compression::GetBlocks(){
	return AllLayerBlocks;
}

size_t Compression::GetBlocksSize(){
	return AllLayerBlocks.size();
}

std::vector<Block>& Compression::GetFinalBlocks(){
	return FinalBlocks;
}
