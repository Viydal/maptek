#ifndef PARSE_H
#define PARSE_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Parse {
public:
  int XCount, YCount, ZCount, ParentX, ParentY, ParentZ;
  std::unordered_map<char, std::string> TagTable;
  std::vector<std::vector<std::string>> MapInformation;
  std::vector<std::vector<std::string>> Blocks;
  std::vector<std::vector<std::vector<std::string>>> ParentBlockInformation;
  int NumXBlocks, NumYBlocks, NumZBlocks;

public:
  Parse();
  Parse(std::vector<std::string> Line);
  std::unordered_map<char, std::string> GetTagTable();
  std::string RLERow(std::string Row);
  std::string *RLERowParent(std::string Row, int ParentX, int NumXBlocks);
  std::vector<std::vector<std::string>> GetMap();
  char GetLetter(std::string Encoded, int Col);
};

#endif