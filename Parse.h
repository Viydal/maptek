#ifndef PARSE_H
#define PARSE_H
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Parse {
public:
  int Xcount, Ycount, Zcount, ParentX, ParentY, ParentZ;
  std::unordered_map<char, std::string> TagTable;

public:
  Parse();
  Parse(std::vector<std::string> line);
  std::unordered_map<char, std::string> getTagTable();
  char GetLetter(std::string encoded, int col);
};

#endif