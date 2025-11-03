
#ifndef COMPRESSION_H
#define COMPRESSION_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Helpers.h"

struct PathCandidate {
    ParentBlock Block;
    int Size;
};

class Compression {
public:
  int XCount, YCount, ZCount, ParentX, ParentY, ParentZ;
  std::vector<Block> AllLayerBlocks;
  std::vector<Block> FinalBlocks;

  Compression();
  void Merge(ParentBlock &ParentBlock);
  void CompressParentBlock(ParentBlock &ParentBlock);
  void TryAllSequences(ParentBlock Current, ParentBlock& Best, int& BestSize, int Depth, int MaxDepth, double OriginalSize);
  void TryAllSequencesHelper(ParentBlock Current, std::vector<PathCandidate>& Candidates, ParentBlock& Best, int& BestSize, int Depth, int MaxDepth);

  void PerfectXY(std::vector<Block> &Blocks, int ParentX, int ParentY, int ParentZ);
  void PerfectZ(std::vector<Block>& Blocks, int ParentZ);
  void PerfectX(std::vector<Block>& Blocks);
  void RelaxedXY(std::vector<Block> &Blocks);
  void RelaxedZ(std::vector<Block> &Blocks);
};

#endif