#include "Compression.h"
#include <algorithm>
#include <chrono>
Compression::Compression() {}

inline void compact_live(std::vector<Block>& v) {
    size_t w = 0;
    for (size_t r = 0, n = v.size(); r < n; ++r) {
        if (!v[r].Merged) {
            if (w != r) v[w] = std::move(v[r]);
            ++w;
        }
    }
    v.resize(w);
}

//deleting the merged parent blocks reducing the number of blocks needed to be iterated over
void Compression::CompressParentBlock(ParentBlock &ParentBlock, int &DeleteTime, int &CompressTime) {

	auto start = std::chrono::high_resolution_clock::now();
	ProcessLayerSort(ParentBlock.Blocks, ParentBlock.LimitX, ParentBlock.LimitY, ParentBlock.LimitZ);
	auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	CompressTime += duration.count();

	start = std::chrono::high_resolution_clock::now();
	compact_live(ParentBlock.Blocks);
	end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DeleteTime += duration.count();

	start = std::chrono::high_resolution_clock::now();
	MergeLayers(ParentBlock.Blocks, ParentBlock.LimitZ);
	end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	CompressTime += duration.count();
	
	start = std::chrono::high_resolution_clock::now();
	compact_live(ParentBlock.Blocks);
	end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DeleteTime += duration.count();

	start = std::chrono::high_resolution_clock::now();
	RelaxedXY(ParentBlock.Blocks);
	end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	CompressTime += duration.count();
	
	start = std::chrono::high_resolution_clock::now();
	compact_live(ParentBlock.Blocks);
	end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DeleteTime += duration.count();
	
	start = std::chrono::high_resolution_clock::now();
	MergeLayers(ParentBlock.Blocks, ParentBlock.LimitZ);
	end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	CompressTime += duration.count();
	
	start = std::chrono::high_resolution_clock::now();
	compact_live(ParentBlock.Blocks);
	end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DeleteTime += duration.count();
}

void Compression::CompressParentBlock(ParentBlock &ParentBlock) {
	ProcessLayerSort(ParentBlock.Blocks, ParentBlock.LimitX, ParentBlock.LimitY, ParentBlock.LimitZ);
	ParentBlock.Blocks.erase(std::remove_if(ParentBlock.Blocks.begin(), ParentBlock.Blocks.end(),[](const Block& b){return b.Merged;}), ParentBlock.Blocks.end());

	MergeLayers(ParentBlock.Blocks, ParentBlock.LimitZ);
	ParentBlock.Blocks.erase(std::remove_if(ParentBlock.Blocks.begin(), ParentBlock.Blocks.end(),[](const Block& b){return b.Merged;}), ParentBlock.Blocks.end());

	RelaxedXY(ParentBlock.Blocks);
	ParentBlock.Blocks.erase(std::remove_if(ParentBlock.Blocks.begin(), ParentBlock.Blocks.end(),[](const Block& b){return b.Merged;}), ParentBlock.Blocks.end());
	
	MergeLayers(ParentBlock.Blocks, ParentBlock.LimitZ);
	ParentBlock.Blocks.erase(std::remove_if(ParentBlock.Blocks.begin(), ParentBlock.Blocks.end(),[](const Block& b){return b.Merged;}), ParentBlock.Blocks.end());
}


void Compression::RelaxedXY(std::vector<Block> &Blocks) {
    std::sort(Blocks.begin(), Blocks.end(), [](const Block& a, const Block& b) {
        if (a.Ch   != b.Ch  ) return a.Ch   < b.Ch;
        if (a.ZPos != b.ZPos) return a.ZPos < b.ZPos;
        if (a.YPos != b.YPos) return a.YPos < b.YPos;
        if (a.XPos != b.XPos) return a.XPos < b.XPos;
        return a.YPos < b.YPos;
    });

    const size_t Size = Blocks.size();
    std::vector<int> RecheckI;
    RecheckI.reserve(Size / 10);

    for (size_t i = 0; i < Size; i++) {
        Block &Current = Blocks[i];

        while (i + 1 < Size) {
            Block &Next = Blocks[i + 1];

            if (Current.Ch != Next.Ch) break;
            if (Current.ZPos != Next.ZPos || Current.ZSize != Next.ZSize) break;
            if (Current.YPos + Current.YSize != Next.YPos) break;

            int startMerge = std::max(Current.XPos, Next.XPos);
            int EndMerge   = std::min(Current.XPos + Current.XSize, Next.XPos + Next.XSize);
            int overlap    = EndMerge - startMerge;
            if (overlap <= 0) break;
            if (overlap < Current.XSize / 2) break;

            int CurXPosInitial = Current.XPos;
            int CurXSizeInitial = Current.XSize;
            int CurYSizeInitial = Current.YSize;
            int NextXPosInitial = Next.XPos;
            int NextXSizeInitial = Next.XSize;
            int NextYSizeInitial = Next.YSize;

            bool LeftAligned  = (startMerge == Current.XPos && startMerge == Next.XPos);
            bool RightAligned = (EndMerge   == Current.XPos + Current.XSize && EndMerge   == Next.XPos   + Next.XSize);

            if (LeftAligned) {
                if (NextXSizeInitial == overlap && CurXSizeInitial == overlap) {
                    Current.YSize += NextYSizeInitial;
                    Next.Merged = true;
                } else if (NextXSizeInitial == overlap) {
                    Current.XSize = overlap;
                    Current.YSize = CurYSizeInitial + NextYSizeInitial;

                    Next.XPos  = CurXPosInitial + overlap;
                    Next.XSize = CurXSizeInitial - overlap;
                    Next.YPos  = Current.YPos;  Next.ZPos  = Current.ZPos;
                    Next.YSize = CurYSizeInitial; Next.ZSize = Current.ZSize;
                    Next.Ch    = Current.Ch;

                    RecheckI.push_back(int(i + 1));
                } else {
                    Current.XSize = overlap;
                    Current.YSize = CurYSizeInitial + NextYSizeInitial;

                    Next.XPos  = NextXPosInitial + overlap;
                    Next.XSize = NextXSizeInitial - overlap;

                    RecheckI.push_back(int(i + 1));
                }
                i++;
                continue;
            }

            if (RightAligned) {
                if (NextXSizeInitial == overlap && CurXSizeInitial == overlap) {
                    Current.YSize += NextYSizeInitial;
                    Next.Merged = true;
                } else if (NextXSizeInitial == overlap) {
                    Current.XPos  = startMerge;
                    Current.XSize = overlap;
                    Current.YSize = CurYSizeInitial + NextYSizeInitial;

                    Next.XPos  = CurXPosInitial;
                    Next.XSize = startMerge - CurXPosInitial;
                    Next.YPos  = Current.YPos;  Next.ZPos  = Current.ZPos;
                    Next.YSize = CurYSizeInitial; Next.ZSize = Current.ZSize;
                    Next.Ch    = Current.Ch;

                    RecheckI.push_back(int(i + 1));
                } else {
                    Current.YSize = CurYSizeInitial + NextYSizeInitial;

                    Next.XSize = startMerge - NextXPosInitial;

                    RecheckI.push_back(int(i + 1));
                }
                i++;
                continue;
            }

            break;
        }
    }

    for (size_t pos : RecheckI) {
        Block &Current = Blocks[size_t(pos)];

        for (int i = 0; i < Size; ++i) {
            Block &Next = Blocks[i];

            bool canMerge = (Current.Ch == Next.Ch
			&& Current.XPos == Next.XPos && Current.XSize == Next.XSize
			&& Current.ZPos == Next.ZPos&& Current.ZSize == Next.ZSize
			&& (Next.YPos == Current.YPos + Current.YSize));

            if (canMerge) {
                Current.YSize += Next.YSize;
                Next.Merged = true;
            }
        }
    }
}

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
				OutputStack.emplace_back(prev.XPos, prev.YPos, prev.ZPos, prev_left, prev.YSize, prev.ZSize, prev.Ch);
				//std::cout << "Prev left Output: (" << left.XPos << "," << left.YPos << "," << left.ZPos << ") size (" << left.XSize << "," << left.YSize << "," << left.ZSize << ") - " << left.Ch << "\n";
		} else if (prev_right > 0) {
				OutputStack.emplace_back(EndMerge, prev.YPos, prev.ZPos, prev_right, prev.YSize, prev.ZSize, prev.Ch);
				//std::cout << "Prev right Output: (" << right.XPos << "," << right.YPos << "," << right.ZPos << ") size (" << right.XSize << "," << right.YSize << "," << right.ZSize << ") - " << right.Ch << "\n";
		}

		// --- perform the merge using overlap region
		prev.XPos  = StartMerge;
		prev.XSize = overlap;
		prev.YSize += curr.YSize;

		if (curr_left > 0) {
				BlockStack.emplace_back(curr.XPos, curr.YPos, curr.ZPos, curr_left, curr.YSize, curr.ZSize, curr.Ch);
				//std::cout << "Curr left Block Stack: (" << left.XPos << "," << left.YPos << "," << left.ZPos << ") size (" << left.XSize << "," << left.YSize << "," << left.ZSize << ") - " << left.Ch << "\n";
		} else if (curr_right > 0) {
				BlockStack.emplace_back(EndMerge, curr.YPos, curr.ZPos, curr_right, curr.YSize, curr.ZSize, curr.Ch);
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
			Current.ZSize += Next.ZSize;
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
			LeftOvers.emplace_back(Current.XPos, Current.YPos, Current.ZPos, CurrentLeft, Current.YSize, Current.ZSize, Current.Ch);
	} else if (CurrentRight > 0) {
			LeftOvers.emplace_back(EndMerge, Current.YPos, Current.ZPos, CurrentRight, Current.YSize, Current.ZSize, Current.Ch);
	}

	// --- perform the merge using overlap region
	Current.XPos = StartMerge;
	Current.XSize = overlap;
	Current.ZSize += Next.ZSize;

	if (NextLeft > 0) {
			LeftOvers.emplace_back(Next.XPos, Next.YPos, Next.ZPos, NextLeft, Next.YSize, Next.ZSize, Next.Ch);
	} else if (NextRight > 0) {
			LeftOvers.emplace_back(EndMerge, Next.YPos, Next.ZPos, NextRight, Next.YSize, Next.ZSize, Next.Ch);
	}

	return true;
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
					const Block& Next = Blocks[i + 1];
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
