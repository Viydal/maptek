#ifndef COMPRESSION_H
#define COMPRESSION_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct Block {
    int x_pos, y_pos, z_pos;
    int x_size, y_size, z_size;
    char ch;
};

class Compression {
public:
  int Xcount, Ycount, Zcount, ParentX, ParentY, ParentZ;
  std::unordered_map<char, std::string> TagTable;
  std::vector<std::vector<std::string>> MapInformation;

  Compression();
  std::string SingleLineCompress(std::string Row, std::unordered_map<char, std::string> TagTable, int ParentX = 0, int ParentY = 0, int ParentZ = 0, int row_num = 0, int layer_num = 0);
  void FormatOutput(std::ostringstream& output, int x_pos, int row_num, int layer_num, int numX, int numY, int numZ, char ch, std::unordered_map<char, std::string> TagTable);
  std::vector<Block> SingleLineBlocks(const std::string Row, int ParentX, int ParentY, int ParentZ, int row_num, int layer_num);
  std::vector<Block> MergeRows(const std::vector<Block>& prevRow, const std::vector<Block>& currRow, int ParentY);
  void WriteBlocks(const std::vector<Block>& blocks, std::ostringstream& output, const std::unordered_map<char, std::string>& TagTable);
  void ProcessLayer(const std::vector<std::string>& rows, int ParentX, int ParentY, int ParentZ, int layer_num, std::ostringstream& output, const std::unordered_map<char, std::string>& TagTable);
  std::string FormatOutputStrings(std::ostringstream& output, int x_pos, int row_num, int layer_num, int numX, int numY, int numZ, char ch, std::unordered_map<char, std::string> TagTable);
  std::vector<std::string> WriteBlocksVectorStrings(const std::vector<Block>& blocks, std::ostringstream& output, const std::unordered_map<char, std::string>& TagTable);
};

#endif