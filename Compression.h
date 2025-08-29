#ifndef COMPRESSION_H
#define COMPRESSION_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct Block {
  int XPos, YPos, ZPos;
  int XSize, YSize, ZSize;
  char Ch;
};

class Compression {
public:
  int XCount, YCount, ZCount, ParentX, ParentY, ParentZ;
  std::unordered_map<char, std::string> TagTable;
  std::vector<std::vector<std::string>> MapInformation;

  Compression();
  std::string SingleLineCompress(std::string Row, std::unordered_map<char, std::string> TagTable, int ParentX = 0, int ParentY = 0, int ParentZ = 0, int RowNum = 0, int LayerNum = 0);
  std::string FormatOutput(int XPos, int RowNum, int LayerNum, int NumX, int NumY, int NumZ, char Ch, std::unordered_map<char, std::string> TagTable);
  std::vector<Block> SingleLineBlocks(const std::string Row, int ParentX, int ParentY, int ParentZ, int RowNum, int LayerNum);
  std::vector<Block> MergeRows(const std::vector<Block> &PrevRow, const std::vector<Block> &CurrRow, int ParentY);
  std::string WriteBlocks(const std::vector<Block> &Blocks, const std::unordered_map<char, std::string> &TagTable);
  std::string ProcessLayer(const std::vector<std::string> &Rows, int ParentX, int ParentY, int ParentZ, int LayerNum, const std::unordered_map<char, std::string> &TagTable);
  // Not currently in use
  // std::string FormatOutputStrings(std::ostringstream &Output, int XPos, int RowNum, int LayerNum, int NumX, int NumY, int NumZ, char Ch, std::unordered_map<char, std::string> TagTable);
  // std::vector<std::string> WriteBlocksVectorStrings(const std::vector<Block> &Blocks, std::ostringstream &Output, const std::unordered_map<char, std::string> &TagTable);
};

#endif