#ifndef PARSE_H
#define PARSE_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Parse {
public:
  int Xcount, Ycount, Zcount, ParentX, ParentY, ParentZ;
  std::unordered_map<char, std::string> TagTable;
  std::vector<std::vector<std::string>> MapInformation;

public:
  Parse();
  Parse(std::vector<std::string> Line);
  std::unordered_map<char, std::string> getTagTable();
  char GetLetter(std::string encoded, int col);
  std::string RLERow(std::string Row);
  std::vector<std::vector<std::string>> GetMap();
  char GetLetter(std::string encoded, int col);
};

#endif