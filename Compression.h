
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

  Compression();
  void CompressParentBlock(ParentBlock &ParentBlock);
  void ProcessLayerSort(std::vector<Block> &Blocks, int ParentX, int ParentY, int ParentZ);
  void MergeLayers(std::vector<Block>& Blocks, int ParentZ);

  void RelaxedXY(std::vector<Block> &Blocks);
  void RelaxedZ(std::vector<Block> &Blocks);
  void ProcessLayer(std::vector<std::vector<Block>> &Rows, int ParentX, int ParentY, int ParentZ, int LayerNum, std::ostringstream &Output, const std::string* TagTable);
  bool TryRelaxedMerge(Block& prev, Block& curr, int ParentY, std::vector<Block>& BlockStack, std::vector<Block>& OutputStack);
  void MergeRows(std::vector<Block> &OutputStack, std::vector<Block> &Cr, std::vector<Block> &BlockStack, int ParentY);
  
  bool TryRelaxedLayerMerge(Block& Current, Block& Next, int ParentZ, std::vector<Block>& LeftOvers);
  bool TryRelaxedLayer(Block& prev, Block& curr, int ParentZ);

  std::vector<Block>& GetBlocks();
  size_t GetBlocksSize();
  std::vector<Block>& GetFinalBlocks();
};

#endif
