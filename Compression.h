#ifndef COMPRESSION_H
#define COMPRESSION_H
#include "Parse.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Compression {
public:
  int Xcount, Ycount, Zcount, ParentX, ParentY, ParentZ;
  std::unordered_map<char, std::string> TagTable;
  std::vector<std::vector<std::string>> MapInformation;

  Compression();
  std::string SingleLineCompress(std::string Row, std::unordered_map<char, std::string> TagTable, int ParentX = 0, int ParentY = 0, int ParentZ = 0, int x_pos = 0, int row_num = 0, int layer_num = 0);
  void FormatOutput(std::ostringstream& output, int x_pos, int row_num, int layer_num, int num, char ch, std::unordered_map<char, std::string> TagTable);
  void BetterFormat(int x_pos, int y_pos, int layer_num, int size_x, int size_y, int size_z, char location, std::unordered_map<char, std::string> TagTable);
  void Uncompress2d(std::vector<std::string> output, std::unordered_map<char, std::string> TagTable,int Xcount, int Ycount);
  void TwoDCompression(Parse parser, std::vector<std::vector<std::string>> block, size_t row);
};

#endif