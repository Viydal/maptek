
#ifndef COMPRESSION_H
#define COMPRESSION_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Block.h"

class Compression {
public:
  int XCount, YCount, ZCount, ParentX, ParentY, ParentZ;
  std::string * TagTable;
  std::vector<std::vector<std::string>> MapInformation;
  std::vector<Block> AllLayerBlocks;
  std::vector<Block> FinalBlocks;


  Compression();
  std::string SingleLineCompress(std::string Row, std::string* TagTable, int ParentX = 0, int ParentY = 0, int ParentZ = 0, int RowNum = 0, int LayerNum = 0);
  void FormatOutput(std::ostringstream &Output, int XPos, int RowNum,int LayerNum, int NumX, int NumY, int NumZ, char Ch, const std::string* TagTable);
  std::vector<Block> SingleLineBlocks(const std::string Row, int ParentX, int ParentY, int ParentZ, int RowNum, int LayerNum);
  void MergeRows(std::vector<Block> &Output, std::vector<Block> &CurrRow, std::vector<Block> &BlockStack, int ParentY);
  void WriteBlocks(std::vector<Block>& Blocks, std::ostringstream &Output, const std::string*TagTable);
  void WriteBlocksString(std::vector<Block>& Blocks, std::ostringstream &Output, const std::string*TagTable);
  void ProcessLayer(const std::vector<std::vector<Block>> &Rows, int ParentX, int ParentY, int ParentZ, int LayerNum, std::ostringstream &Output, const std::string* TagTable);
  std::string FormatOutputStrings(std::ostringstream &Output, int XPos, int RowNum, int LayerNum, int NumX, int NumY, int NumZ, char Ch, const std::string* TagTable);
  std::vector<std::string> WriteBlocksVectorStrings(const std::vector<Block> &Blocks, std::ostringstream &Output, const std::string * TagTable);
  bool TryRelaxedMerge(Block& prev, Block& curr, int ParentY, std::vector<Block>& BlockStack, std::vector<Block>& OutputStack);
  bool TryRelaxedLayerMerge(Block& Current, Block& Next, int ParentZ, std::vector<Block>& LeftOvers);
  void MergeLayers(std::vector<Block>& Blocks, int ParentZ);
  bool TryRelaxedLayer(Block& prev, Block& curr, int ParentZ);
  std::vector<Block>& GetBlocks();
  size_t GetBlocksSize();
  std::vector<Block>& GetFinalBlocks();
  void FormatSubmit(std::vector<Block> &OutputBlocks);

};

#endif
