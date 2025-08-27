#ifndef MULTILINECOMPRESSION_H
#define MULTILINECOMPRESSION_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class MultiLineCompression {
public:
  int Xcount, Ycount, Zcount, ParentX, ParentY, ParentZ;
  std::unordered_map<char, std::string> TagTable;
  std::vector<std::vector<std::string>> MapInformation;

  MultiLineCompression();
  std::string MultiLineCompress(std::vector<std::string> output, std::unordered_map<char, std::string> TagTable,int Xcount, int Ycount, int ParentY);
};

#endif