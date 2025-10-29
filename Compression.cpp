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

inline void SwapXY(ParentBlock& PB) {
    for (auto& b : PB.Blocks) {
        std::swap(b.XPos, b.YPos);
        std::swap(b.XSize, b.YSize);
    }
    std::swap(PB.LimitX, PB.LimitY);
}

inline void SwapYZ(ParentBlock& pb) {
    for (auto& b : pb.Blocks) {
        std::swap(b.YPos, b.ZPos);
        std::swap(b.YSize, b.ZSize);
    }
    std::swap(pb.LimitY, pb.LimitZ);
}

inline void SwapXZ(ParentBlock& pb) {
    for (auto& b : pb.Blocks) {
        std::swap(b.XPos, b.ZPos);
        std::swap(b.XSize, b.ZSize);
    }
    std::swap(pb.LimitX, pb.LimitZ);
}

inline void RotateYZX(ParentBlock& pb) {
    for (auto& b : pb.Blocks) {
        std::swap(b.XPos, b.YPos);
        std::swap(b.XSize, b.YSize);
        std::swap(b.YPos, b.ZPos);
        std::swap(b.YSize, b.ZSize);
    }
    std::swap(pb.LimitX, pb.LimitY);
    std::swap(pb.LimitY, pb.LimitZ);
}

inline void RotateZXY(ParentBlock& PB) {
    for (auto& B : PB.Blocks) {
        std::swap(B.YPos, B.ZPos);
        std::swap(B.YSize, B.ZSize);
        std::swap(B.XPos, B.YPos);
        std::swap(B.XSize, B.YSize);
    }
    std::swap(PB.LimitY, PB.LimitZ);
    std::swap(PB.LimitX, PB.LimitY);
}

inline void MirrorX(ParentBlock& pb) {
    for (auto& b : pb.Blocks) {
        b.XPos = pb.LimitX - b.XPos - b.XSize;
    }
}

inline void MirrorY(ParentBlock& pb) {
    for (auto& b : pb.Blocks) {
        b.YPos = pb.LimitY - b.YPos - b.YSize;
    }
}

inline void MirrorZ(ParentBlock& pb) {
    for (auto& b : pb.Blocks) {
        b.ZPos = pb.LimitZ - b.ZPos - b.ZSize;
    }
}

inline void MirrorXY(ParentBlock& pb) {
    MirrorX(pb);
    MirrorY(pb);
}

inline void MirrorXZ(ParentBlock& pb) {
    MirrorX(pb);
    MirrorZ(pb);
}

inline void MirrorYZ(ParentBlock& pb) {
    MirrorY(pb);
    MirrorZ(pb);
}

inline void MirrorXYZ(ParentBlock& pb) {
    MirrorX(pb);
    MirrorY(pb);
    MirrorZ(pb);
}

inline void CheckAndSave(ParentBlock &PB, int &PrevSize, ParentBlock &Best, const ParentBlock& Original) {
	if (PB.Blocks.size() < PrevSize) {
		PrevSize = PB.Blocks.size();
		Best = PB;
	}
	PB = Original;
}

void Compression::Merge(ParentBlock &WorkingParentBlock){
	int PrevSize;
	int Strike = 0;

	// Perfect merges only
	PrevSize = WorkingParentBlock.Blocks.size();
	while(true){
		PerfectXY(WorkingParentBlock.Blocks, WorkingParentBlock.LimitX, WorkingParentBlock.LimitY, WorkingParentBlock.LimitZ);
		compact_live(WorkingParentBlock.Blocks);
		
		PerfectZ(WorkingParentBlock.Blocks, WorkingParentBlock.LimitZ);
		compact_live(WorkingParentBlock.Blocks);

		PerfectX(WorkingParentBlock.Blocks);
		compact_live(WorkingParentBlock.Blocks);

		int NewSize = WorkingParentBlock.Blocks.size();
		if (NewSize == PrevSize) {
			Strike++;
		} else {
			Strike = 0;
		}
		if (Strike == 5) break;
		PrevSize = NewSize;
	}

	Strike = 0;
	
	// Relaxed merges only
	PrevSize = WorkingParentBlock.Blocks.size();
	while (true) {
		RelaxedXY(WorkingParentBlock.Blocks);
		compact_live(WorkingParentBlock.Blocks);

		RelaxedZ(WorkingParentBlock.Blocks);
		compact_live(WorkingParentBlock.Blocks);

		int NewSize = WorkingParentBlock.Blocks.size();
		if (NewSize == PrevSize) {
			Strike++;
		} else {
			Strike = 0;
		}
		if (Strike == 5) break;
		PrevSize = NewSize;
	}
}

void Compression::CompressParentBlock(ParentBlock &WorkingParentBlock) {
	int PrevSize = WorkingParentBlock.Blocks.size();

	while (true) {
		ParentBlock Original = WorkingParentBlock;
		ParentBlock CurrentBest = Original;
		ParentBlock WorkingCopy = WorkingParentBlock;

		//(x,y,z)
        Merge(WorkingCopy);
		// << "X Y Z DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);

		//(x,z,y)
        SwapYZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapYZ(WorkingCopy);
		// << "X Z Y DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,x,z)
        SwapXY(WorkingCopy);
        Merge(WorkingCopy);
        SwapXY(WorkingCopy);
		// << "Y X Z DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,z,x)
        RotateYZX(WorkingCopy);
        Merge(WorkingCopy);
        RotateZXY(WorkingCopy);
		// << "Y Z X DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // XZY rotation (z,x,y)
        RotateZXY(WorkingCopy);
        Merge(WorkingCopy);
        RotateYZX(WorkingCopy);
		// << "Z X Y DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // (z,y,x)
        SwapXZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapXZ(WorkingCopy);
		// << "Z Y X DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
		// << std::endl;

		// Mirror Z operations

		//(x,y,z)
		MirrorZ(WorkingCopy);
        Merge(WorkingCopy);
		MirrorZ(WorkingCopy);
		// << "X Y Z MIRROR Z DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);

		//(x,z,y)
		MirrorZ(WorkingCopy);
        SwapYZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapYZ(WorkingCopy);
		MirrorZ(WorkingCopy);
		// << "X Z Y MIRROR Z DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,x,z)
		MirrorZ(WorkingCopy);
        SwapXY(WorkingCopy);
        Merge(WorkingCopy);
        SwapXY(WorkingCopy);
		MirrorZ(WorkingCopy);
		// << "Y X Z MIRROR Z DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,z,x)
		MirrorZ(WorkingCopy);
        RotateYZX(WorkingCopy);
        Merge(WorkingCopy);
        RotateZXY(WorkingCopy);
		MirrorZ(WorkingCopy);
		// << "Y Z X MIRROR Z DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // XZY rotation (z,x,y)
		MirrorZ(WorkingCopy);
        RotateZXY(WorkingCopy);
        Merge(WorkingCopy);
        RotateYZX(WorkingCopy);
		MirrorZ(WorkingCopy);
		// << "Z X Y MIRROR Z DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // (z,y,x)
		MirrorZ(WorkingCopy);
        SwapXZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapXZ(WorkingCopy);
		MirrorZ(WorkingCopy);
		// << "Z Y X MIRROR Z DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
		// << std::endl;

		// Mirror Y operations
		
		//(x,y,z)
		MirrorY(WorkingCopy);
        Merge(WorkingCopy);
		MirrorY(WorkingCopy);
		// << "X Y Z MIRROR Y DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);

		//(x,z,y)
		MirrorY(WorkingCopy);
        SwapYZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapYZ(WorkingCopy);
		MirrorY(WorkingCopy);
		// << "X Z Y MIRROR Y DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,x,z)
		MirrorY(WorkingCopy);
        SwapXY(WorkingCopy);
        Merge(WorkingCopy);
        SwapXY(WorkingCopy);
		MirrorY(WorkingCopy);
		// << "Y X Z MIRROR Y DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,z,x)
		MirrorY(WorkingCopy);
        RotateYZX(WorkingCopy);
        Merge(WorkingCopy);
        RotateZXY(WorkingCopy);
		MirrorY(WorkingCopy);
		// << "Y Z X MIRROR Y DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // XZY rotation (z,x,y)
		MirrorY(WorkingCopy);
        RotateZXY(WorkingCopy);
        Merge(WorkingCopy);
        RotateYZX(WorkingCopy);
		MirrorY(WorkingCopy);
		// << "Z X Y MIRROR Y DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // (z,y,x)
		MirrorY(WorkingCopy);
        SwapXZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapXZ(WorkingCopy);
		MirrorY(WorkingCopy);
		// << "Z Y X MIRROR Y DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
		// << std::endl;

		// Mirror X operations
		
		//(x,y,z)
		MirrorX(WorkingCopy);
        Merge(WorkingCopy);
		MirrorX(WorkingCopy);
		// << "X Y Z MIRROR X DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);

		//(x,z,y)
		MirrorX(WorkingCopy);
        SwapYZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapYZ(WorkingCopy);
		MirrorX(WorkingCopy);
		// << "X Z Y MIRROR X DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,x,z)
		MirrorX(WorkingCopy);
        SwapXY(WorkingCopy);
        Merge(WorkingCopy);
        SwapXY(WorkingCopy);
		MirrorX(WorkingCopy);
		// << "Y X Z MIRROR X DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,z,x)
		MirrorX(WorkingCopy);
        RotateYZX(WorkingCopy);
        Merge(WorkingCopy);
        RotateZXY(WorkingCopy);
		MirrorX(WorkingCopy);
		// << "Y Z X MIRROR X DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // XZY rotation (z,x,y)
		MirrorX(WorkingCopy);
        RotateZXY(WorkingCopy);
        Merge(WorkingCopy);
        RotateYZX(WorkingCopy);
		MirrorX(WorkingCopy);
		// << "Z X Y MIRROR X DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // (z,y,x)
		MirrorX(WorkingCopy);
        SwapXZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapXZ(WorkingCopy);
		MirrorX(WorkingCopy);
		// << "Z Y X MIRROR X DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
		// << std::endl;

		// Mirror XY operations
		
		//(x,y,z)
		MirrorXY(WorkingCopy);
        Merge(WorkingCopy);
		MirrorXY(WorkingCopy);
		// << "X Y Z MIRROR XY DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);

		//(x,z,y)
		MirrorXY(WorkingCopy);
        SwapYZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapYZ(WorkingCopy);
		MirrorXY(WorkingCopy);
		// << "X Z Y MIRROR XY DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,x,z)
		MirrorXY(WorkingCopy);
        SwapXY(WorkingCopy);
        Merge(WorkingCopy);
        SwapXY(WorkingCopy);
		MirrorXY(WorkingCopy);
		// << "Y X Z MIRROR XY DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        //(y,z,x)
		MirrorXY(WorkingCopy);
        RotateYZX(WorkingCopy);
        Merge(WorkingCopy);
        RotateZXY(WorkingCopy);
		MirrorXY(WorkingCopy);
		// << "Y Z X MIRROR XY DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // XZY rotation (z,x,y)
		MirrorXY(WorkingCopy);
        RotateZXY(WorkingCopy);
        Merge(WorkingCopy);
        RotateYZX(WorkingCopy);
		MirrorXY(WorkingCopy);
		// << "Z X Y MIRROR XY DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
        
        // (z,y,x)
		MirrorXY(WorkingCopy);
        SwapXZ(WorkingCopy);
        Merge(WorkingCopy);
        SwapXZ(WorkingCopy);
		MirrorXY(WorkingCopy);
		// << "Z Y X MIRROR XY DONE, number of blocks: " << WorkingCopy.Blocks.size() << std::endl;
		CheckAndSave(WorkingCopy, PrevSize, CurrentBest, Original);
		// << std::endl;

        int NewSize = WorkingCopy.Blocks.size();
        if (NewSize == PrevSize) break;
        PrevSize = NewSize;
		WorkingParentBlock = CurrentBest;
    }
}


void Compression::RelaxedXY(std::vector<Block> &Blocks) {
    const size_t Size = Blocks.size();
    if (Size < 2) return;
    
    std::sort(Blocks.begin(), Blocks.end(), [](const Block& a, const Block& b) {
        if (a.Ch   != b.Ch  ) return a.Ch   < b.Ch;
        if (a.ZPos != b.ZPos) return a.ZPos < b.ZPos;
        if (a.YPos != b.YPos) return a.YPos < b.YPos;
        return a.XPos < b.XPos;
    });

    for (size_t i = 0; i < Size; ) {
        if (Blocks[i].Merged) {
            i++;
            continue;
        }
        
        Block &Current = Blocks[i];
        bool merged = false;

        for (size_t j = i + 1; j < Size; j++) {
            if (Blocks[j].Merged) continue;
            Block &Next = Blocks[j];

            // Early exit - blocks are sorted by Ch, ZPos, YPos
            if (Current.Ch != Next.Ch) break;
            if (Current.ZPos != Next.ZPos || Current.ZSize != Next.ZSize) break;
            
            // Check if Next is vertically adjacent to Current
            if (Current.YPos + Current.YSize != Next.YPos) {
                // If Next.YPos is beyond Current's end, no more blocks can merge
                if (Next.YPos > Current.YPos + Current.YSize) break;
                continue;
            }

            // Calculate overlap
            int startMerge = std::max(Current.XPos, Next.XPos);
            int EndMerge   = std::min(Current.XPos + Current.XSize, Next.XPos + Next.XSize);
            int overlap    = EndMerge - startMerge;
            
            if (overlap <= 0) continue;
            if (overlap < Current.XSize / 2) continue;

            int CurXPosInitial = Current.XPos;
            int CurXSizeInitial = Current.XSize;
            int CurYSizeInitial = Current.YSize;
            int NextXPosInitial = Next.XPos;
            int NextXSizeInitial = Next.XSize;
            int NextYSizeInitial = Next.YSize;

            bool LeftAligned  = (startMerge == Current.XPos && startMerge == Next.XPos);
            bool RightAligned = (EndMerge   == Current.XPos + Current.XSize && EndMerge == Next.XPos + Next.XSize);

            if (LeftAligned) {
                if (NextXSizeInitial == overlap && CurXSizeInitial == overlap) {
                    // Perfect merge
                    Current.YSize += NextYSizeInitial;
                    Next.Merged = true;
                    merged = true;
                } else if (NextXSizeInitial == overlap) {
                    // Merge overlap, keep Current remainder
                    Current.XSize = overlap;
                    Current.YSize = CurYSizeInitial + NextYSizeInitial;

                    Next.XPos  = CurXPosInitial + overlap;
                    Next.XSize = CurXSizeInitial - overlap;
                    Next.YPos  = Current.YPos;
                    Next.ZPos  = Current.ZPos;
                    Next.YSize = CurYSizeInitial;
                    Next.ZSize = Current.ZSize;
                    Next.Ch    = Current.Ch;
                    Next.Merged = false; // Will be processed later
                    merged = true;
                } else {
                    // Merge overlap, keep Next remainder
                    Current.XSize = overlap;
                    Current.YSize = CurYSizeInitial + NextYSizeInitial;

                    Next.XPos  = NextXPosInitial + overlap;
                    Next.XSize = NextXSizeInitial - overlap;
                    Next.Merged = false; // Will be processed later
                    merged = true;
                }
                continue; // Check for more blocks to merge with Current
            }

            if (RightAligned) {
                if (NextXSizeInitial == overlap && CurXSizeInitial == overlap) {
                    // Perfect merge
                    Current.YSize += NextYSizeInitial;
                    Next.Merged = true;
                    merged = true;
                } else if (NextXSizeInitial == overlap) {
                    // Merge overlap, keep Current remainder
                    Current.XPos  = startMerge;
                    Current.XSize = overlap;
                    Current.YSize = CurYSizeInitial + NextYSizeInitial;

                    Next.XPos  = CurXPosInitial;
                    Next.XSize = startMerge - CurXPosInitial;
                    Next.YPos  = Current.YPos;
                    Next.ZPos  = Current.ZPos;
                    Next.YSize = CurYSizeInitial;
                    Next.ZSize = Current.ZSize;
                    Next.Ch    = Current.Ch;
                    Next.Merged = false; // Will be processed later
                    merged = true;
                } else {
                    // Merge overlap, keep Next remainder
                    Current.YSize = CurYSizeInitial + NextYSizeInitial;
                    Next.XSize = startMerge - NextXPosInitial;
                    Next.Merged = false; // Will be processed later
                    merged = true;
                }
                continue; // Check for more blocks to merge with Current
            }

            // No alignment, can't merge
            break;
        }
        
        i++;
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
    const size_t Size = Blocks.size();
    if (Size < 2) return;
    
    // Sort once
    std::sort(Blocks.begin(), Blocks.end(),[](const Block& a, const Block& b) {
        if (a.Ch    != b.Ch  ) return a.Ch   < b.Ch;
        if (a.XPos  != b.XPos) return a.XPos < b.XPos;
        if (a.YPos  != b.YPos) return a.YPos < b.YPos;
        return a.ZPos < b.ZPos;
    });

    // Single pass merging
    for (size_t i = 0; i < Size; ) {
        if (Blocks[i].Merged) {
            i++;
            continue;
        }
        
        Block& Current = Blocks[i];
        bool merged = false;
        
        for (size_t j = i + 1; j < Size; j++) {
            if (Blocks[j].Merged) continue;
            Block& Next = Blocks[j];

            // Early exit if we've moved to different Ch/X/Y group
            if (Current.Ch != Next.Ch || Current.XPos != Next.XPos || Current.YPos != Next.YPos) {
                break;
            }

            if (!CanRelaxedZMerge(Current, Next)) continue;
            
            int region = RegionChoice(Current, Next);
            
            if (region == -1) continue;
            
            switch (region) {
                case 0: // Perfect alignment
                    Current.ZSize += Next.ZSize;
                    Next.Merged = true;
                    merged = true;
                    break;
                    
                case 2: // Perfect X, trim Next End Y
                    Current.ZSize += Next.ZSize;
                    Next.YPos = Current.YPos + Current.YSize;
                    Next.YSize -= Current.YSize;
                    Next.Merged = false; // Will be checked later
                    merged = true;
                    break;
                    
                case 4: // Perfect X, trim Current Start Y
                    Next.YSize = Next.YPos - Current.YPos;
                    Current.ZSize += Next.ZSize;
                    Next.Merged = false;
                    merged = true;
                    break;
                    
                case 6: // Perfect Y, trim Next End X
                    Current.ZSize += Next.ZSize;
                    Next.XPos = Current.XPos + Current.XSize;
                    Next.XSize -= Current.XSize;
                    Next.Merged = false;
                    merged = true;
                    break;
                    
                case 8: // Perfect Y, trim Next Start X
                    Current.ZSize += Next.ZSize;
                    Next.XSize = Current.XSize - (Next.XPos - Current.XPos);
                    Next.Merged = false;
                    merged = true;
                    break;
                    
                // Cases where Current is modified - stop looking forward
                case 1: // Perfect X, trim Current End Y
                case 3: // Perfect X, trim Next Start Y
                case 5: // Perfect Y, trim Current End X
                case 7: // Perfect Y, trim Current Start X
                    // Apply transformation and move to next block
                    if (region == 1) {
                        Next.ZPos = Current.ZPos;
                        Next.ZSize += Current.ZSize;
                        Current.YPos = Next.YPos + Next.YSize;
                        Current.YSize -= Next.YSize;
                    } else if (region == 3) {
                        Current.YSize -= Next.YSize;
                        Next.ZPos = Current.ZPos;
                        Next.ZSize += Current.ZSize;
                    } else if (region == 5) {
                        Next.ZSize += Current.ZSize;
                        Next.ZPos = Current.ZPos;
                        Current.XPos = Next.XPos + Next.XSize;
                        Current.XSize -= Next.XSize;
                    } else if (region == 7) {
                        Current.XSize = Next.XPos - Current.XPos;
                        Next.ZSize += Current.ZSize;
                        Next.ZPos = Current.ZPos;
                    }
                    goto next_block;
            }
        }
        
        next_block:
        i++;
    }
}
	
/**
 * PerfectZ
 * -----------
 * Attempts to merge blocks vertically along the Z axis.
 * Blocks can merge if they:
 *   - Have identical X, Y, size, and Ch
 *   - Are adjacent in Z
 *   - Do not exceed ParentZ boundaries
 */
void Compression::PerfectZ(std::vector<Block>& Blocks, int ParentZ) {
	
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

void Compression::PerfectX(std::vector<Block>& Blocks) {
	// Sort all blocks by position (X, Y, then Z)
	std::sort(Blocks.begin(), Blocks.end(), [](const Block& a, const Block& b) {
		if (a.YPos != b.YPos) return a.YPos < b.YPos;
		if (a.ZPos != b.ZPos) return a.ZPos < b.ZPos;
		return a.XPos < b.XPos;
	});

	for (size_t i = 0; i < Blocks.size(); i++) {
		Block& Current = Blocks[i];
		while (i + 1 < Blocks.size()) {
			const Block& Next = Blocks[i + 1];
			// Can the blocks be merged?
			bool canMerge = (Current.YPos == Next.YPos && Current.YSize == Next.YSize && Current.Ch == Next.Ch && Current.ZPos == Next.ZPos && Current.ZSize == Next.ZSize && (Next.XPos == Current.XPos + Current.XSize));
			
			if (!canMerge) break;

			Current.XSize = Current.XSize + Next.XSize;
			Blocks[i+1].Merged = true;
			i++;
		}
	}


}

void Compression::PerfectXY(std::vector<Block> &Blocks, int ParentX, int ParentY, int ParentZ) {

		std::sort(Blocks.begin(), Blocks.end(),[](const Block& a, const Block& b) {
			if (a.XPos  != b.XPos)  return a.XPos  < b.XPos;
			if (a.ZPos  != b.ZPos)  return a.ZPos  < b.ZPos;
			return a.YPos < b.YPos;
		});
		
		for (size_t i = 0; i < Blocks.size(); i++) {
				Block& Current = Blocks[i];
				while (i + 1 < Blocks.size()) {
					const Block& Next = Blocks[i + 1];
					//// << "testinA " << Next.YPos << " " << ParentY << std::endl;
					//// << "After Testing" << (Next.YPos / ParentY) << std::endl;
					bool canMerge = Current.Ch == Next.Ch && Current.XPos == Next.XPos && 
					Current.XSize == Next.XSize && Current.ZPos == Next.ZPos && Current.ZSize == Next.ZSize &&
					(Next.YPos == Current.YPos + Current.YSize);
					//// << "After" << std::endl;
					if (!canMerge) break;

					int TotalYSize = Current.YSize + Next.YSize;
					Current.YSize = TotalYSize;
					Blocks[i+1].Merged = true;
					i++;
				}
		}
}