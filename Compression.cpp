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

void Compression::Merge(ParentBlock &ParentBlock){
	int numBlocks;
	int newNumBlocks;

	while(true){
		numBlocks = ParentBlock.Blocks.size();

		ProcessLayerSort(ParentBlock.Blocks, ParentBlock.LimitX, ParentBlock.LimitY, ParentBlock.LimitZ);
		compact_live(ParentBlock.Blocks);

		MergeLayers(ParentBlock.Blocks, ParentBlock.LimitZ);
		compact_live(ParentBlock.Blocks);

		RelaxedXY(ParentBlock.Blocks);
		compact_live(ParentBlock.Blocks);

		RelaxedZ(ParentBlock.Blocks);
		compact_live(ParentBlock.Blocks);

		newNumBlocks = ParentBlock.Blocks.size();
		if (newNumBlocks == numBlocks)
		{
			break;
		}
	}
}



void Compression::CompressParentBlock(ParentBlock &ParentBlock) {

	// x, y, z
	
	Merge(ParentBlock);

	// yz swap (x, z, y)

	for (auto& b : ParentBlock.Blocks) {
        std::swap(b.YPos,  b.ZPos);
        std::swap(b.YSize, b.ZSize);
    }
	std::swap(ParentBlock.LimitY, ParentBlock.LimitZ);

	Merge(ParentBlock);

	std::swap(ParentBlock.LimitY, ParentBlock.LimitZ);
	for (auto& b : ParentBlock.Blocks) {
        std::swap(b.YPos,  b.ZPos);
        std::swap(b.YSize, b.ZSize);
    }

	// rotate (y,x,z)
	
	for (auto& b : ParentBlock.Blocks) {
        std::swap(b.XPos,  b.YPos);
        std::swap(b.XSize, b.YSize);
    }
	std::swap(ParentBlock.LimitX, ParentBlock.LimitY);

	Merge(ParentBlock);

	std::swap(ParentBlock.LimitX, ParentBlock.LimitY);
	for (auto& b : ParentBlock.Blocks) {
        std::swap(b.XPos,  b.YPos);
        std::swap(b.XSize, b.YSize);
    }

	// rotate (y,z,x)

	for (auto& b : ParentBlock.Blocks) {
        std::swap(b.XPos,  b.YPos);
        std::swap(b.XSize, b.YSize);

		std::swap(b.YPos,  b.ZPos);
        std::swap(b.YSize, b.ZSize);
    }
	std::swap(ParentBlock.LimitX, ParentBlock.LimitY);
	std::swap(ParentBlock.LimitY, ParentBlock.LimitZ);

	Merge(ParentBlock);

	std::swap(ParentBlock.LimitY, ParentBlock.LimitZ);
	std::swap(ParentBlock.LimitX, ParentBlock.LimitY);
	for (auto& b : ParentBlock.Blocks) {
		std::swap(b.YPos,  b.ZPos);
        std::swap(b.YSize, b.ZSize);

        std::swap(b.XPos,  b.YPos);
        std::swap(b.XSize, b.YSize);
    }

	// rotate (z,x,y) -- breaks it idk why
	
	for (auto& b : ParentBlock.Blocks) {
        std::swap(b.XPos,  b.ZPos);
        std::swap(b.XSize, b.ZSize);

		std::swap(b.YPos,  b.ZPos);
        std::swap(b.YSize, b.ZSize);
    }
	std::swap(ParentBlock.LimitX, ParentBlock.LimitZ);
	std::swap(ParentBlock.LimitY, ParentBlock.LimitZ);

	Merge(ParentBlock);

	std::swap(ParentBlock.LimitY, ParentBlock.LimitZ);
	std::swap(ParentBlock.LimitX, ParentBlock.LimitZ);
	for (auto& b : ParentBlock.Blocks) {
		std::swap(b.YPos,  b.ZPos);
        std::swap(b.YSize, b.ZSize);

        std::swap(b.XPos,  b.ZPos);
        std::swap(b.XSize, b.ZSize);
    }

	// rotate (z,y,x)

	for (auto& b : ParentBlock.Blocks) {
        std::swap(b.XPos,  b.ZPos);
        std::swap(b.XSize, b.ZSize);
    }

	std::swap(ParentBlock.LimitX, ParentBlock.LimitZ);

	Merge(ParentBlock);

	std::swap(ParentBlock.LimitX, ParentBlock.LimitZ);

	for (auto& b : ParentBlock.Blocks) {
        std::swap(b.XPos,  b.ZPos);
        std::swap(b.XSize, b.ZSize);
    }

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
    RecheckI.reserve(Size);

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
		if (Blocks[pos].Merged) continue;
        Block &Current = Blocks[pos];

        for (int i = 0; i < Size; ++i) {
			if (i == pos) continue;
			if (Blocks[i].Merged) continue;
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

bool CanRelaxedZMerge(const Block& Current, const Block& Next) {
	//Base Rule: Blocks can be merged if they have the same X, Y, size, and Ch, and are adjacent in Z
	if (Current.ZPos + Current.ZSize != Next.ZPos) return false;
	if (Current.Ch != Next.Ch) return false;
	return true;
}

int RegionChoice(const Block& A, const Block& B) {
	if (A.XPos == B.XPos && A.XSize == B.XSize && A.YPos == B.YPos && A.YSize == B.YSize) {
		return 0; // perfect alignment
	} else if (A.XPos == B.XPos) {
		if (A.XSize == B.XSize) {
			if (A.YPos == B.YPos) {
				return 1 + (A.YSize < B.YSize); // Perfect X, trim Y (1=A Y, 2=B Y)
			} else if (A.YPos + A.YSize == B.YPos + B.YSize) {
				return 3 + (A.YPos > B.YPos); // Perfect X, End Y - trim start Y (3=A Y, 4=B Y)
			}
		} else {
			if (A.YPos == B.YPos && A.YSize == B.YSize) {
				return 5 + (A.XSize < B.XSize); // Perfect Y, trim X (5=A X, 6=B X)
			} 
		}
	} else {
		if (A.YPos == B.YPos && A.XPos + A.XSize == B.XPos + B.XSize && A.YSize == B.YSize) {
			return 7 + (A.XPos > B.XPos); // Perfect Y, End X - trim start X (7=A X, 8=B X)
		}
	}
	return -1; // no merge possible
}

void Compression::RelaxedZ(std::vector<Block> &Blocks) {
	std::sort(Blocks.begin(), Blocks.end(),[](const Block& a, const Block& b) {
		if (a.Ch    != b.Ch  ) return a.Ch   < b.Ch;
		if (a.XPos  != b.XPos)  return a.XPos  < b.XPos;
		if (a.YPos  != b.YPos)  return a.YPos  < b.YPos;
		if (a.ZPos  != b.ZPos)  return a.ZPos  < b.ZPos;
		return a.YPos < b.YPos;
	});

	std::vector<int> RecheckI;
	int Size = Blocks.size();
    RecheckI.reserve(Size);
	int counter = 0;
	for (size_t i = 0; i < Size; i++) {
		Block& Current = Blocks[i];
		while (i + 1 < Size) {
			Block& Next = Blocks[i + 1];

			if (!CanRelaxedZMerge(Current, Next)) break;
			int region = RegionChoice(Current, Next);
			
			if (region == -1) {
				i++;
				continue;
			}
			
			switch (region) {
				case 0: // perfect alignment
					Current.ZSize += Next.ZSize;
					Next.Merged = true;
					break;
				case 1:{ // Perfect X, trim Current End Y
					Next.ZPos = Current.ZPos;
					Next.ZSize += Current.ZSize;
					Current.YPos = Next.YPos + Next.YSize;
					Current.YSize -= Next.YSize;
					goto BREAK;
				}
				case 2: { // Perfect X, trim Next End Y
					Current.ZSize += Next.ZSize;
					Next.YPos = Current.YPos + Current.YSize;
					Next.YSize -= Current.YSize;
					RecheckI.push_back(int(i + 1));
					break;
				}
				case 3: { // Perfect X, trim Next Start Y
					Current.YSize -= Next.YSize;
					Next.ZPos = Current.ZPos;
					Next.ZSize += Current.ZSize;
					goto BREAK;
				}
				case 4: { // Perfect X, trim Current Start Y
					Next.YSize = Next.YPos - Current.YPos;
					Current.ZSize += Next.ZSize;
					RecheckI.push_back(int(i + 1));
					break;
					
				}
				case 5: { // Perfect Y, trim Current End X
					Next.ZSize += Current.ZSize;
					Next.ZPos = Current.ZPos;
					Current.XPos = Next.XPos + Next.XSize;
					Current.XSize -= Next.XSize;
					goto BREAK;
				}
				case 6: { // Perfect Y, trim Next End X
					Current.ZSize += Next.ZSize;
					Next.XPos = Current.XPos + Current.XSize;
					Next.XSize -= Current.XSize;
					RecheckI.push_back(int(i + 1));
					break;
				}
				case 7: { // Perfect Y, trim Current Start X
					Current.XSize = Next.XPos - Current.XPos;
					Next.ZSize += Current.ZSize;
					Next.ZPos = Current.ZPos;
					goto BREAK;
				}
				case 8:{ // Perfect Y, trim Next Start X
					Current.ZSize += Next.ZSize;
					Next.XSize = Current.XSize - (Next.XPos - Current.XPos);
					RecheckI.push_back(int(i + 1));
					break;
				}
				default:
					break;
			}
			
			i++;
			continue;
			BREAK:
			break;
		}
	}
	for (size_t pos : RecheckI) {
		if (Blocks[pos].Merged) continue;
        Block &Current = Blocks[pos];

        for (int i = 0; i < Size; ++i) {
			if (i == pos) continue;
			if (Blocks[i].Merged) continue;
            Block &Next = Blocks[i];

            bool canMerge = (Current.XPos == Next.XPos && 
							Current.YPos == Next.YPos &&
							Current.XSize == Next.XSize && 
							Current.YSize == Next.YSize &&
							Current.Ch == Next.Ch &&
							Current.ZPos + Current.ZSize == Next.ZPos);

            if (canMerge) {
                Current.ZSize += Next.ZSize;
				Next.Merged = true;
            }
        }
    }
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
					Current.XSize == Next.XSize && Current.ZPos == Next.ZPos && Current.ZSize == Next.ZSize &&
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