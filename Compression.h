
#ifndef COMPRESSION_H
#define COMPRESSION_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Helpers.h"

class Compression {
public:
  int XCount, YCount, ZCount, ParentX, ParentY, ParentZ;
  std::vector<Block> AllLayerBlocks;
  std::vector<Block> FinalBlocks;

  Compression();
  void CompressParentBlock(ParentBlock &ParentBlock);
  void ProcessLayer(std::vector<std::vector<Block>> &Rows, int ParentX, int ParentY, int ParentZ, int LayerNum, std::ostringstream &Output, const std::string* TagTable);
  void ProcessLayerSort(std::vector<Block> &Blocks, int ParentX, int ParentY, int ParentZ);
  void MergeRows(std::vector<Block> &OutputStack, std::vector<Block> &Cr, std::vector<Block> &BlockStack, int ParentY);
  bool TryRelaxedMerge(Block& prev, Block& curr, int ParentY, std::vector<Block>& BlockStack, std::vector<Block>& OutputStack);

  bool TryRelaxedLayerMerge(Block& Current, Block& Next, int ParentZ, std::vector<Block>& LeftOvers);
  void MergeLayers(std::vector<Block>& Blocks, int ParentZ);
  bool TryRelaxedLayer(Block& prev, Block& curr, int ParentZ);

  std::vector<Block>& GetBlocks();
  size_t GetBlocksSize();
  std::vector<Block>& GetFinalBlocks();
};

#endif
