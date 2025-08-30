#ifndef PARSE_H
#define PARSE_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Block.h"

class Parse {
public:
  int XCount, YCount, ZCount, ParentX, ParentY, ParentZ;
  std::string TagTable[256];
  std::vector<std::vector<std::vector<Block>>> XBlocks;
  std::vector<std::vector<std::string>> MapInformation;
  int NumXBlocks, NumYBlocks, NumZBlocks;

public:
  Parse();
  Parse(std::vector<std::string> Line);
  std::string * GetTagTable() {return TagTable;};
  std::string TestRLERow(std::string Row);
  void RLERow(std::string XBlockString, std::vector<Block> *RowBlocks,std::unordered_map<std::string, std::vector<std::pair<int,char>>> *DP,int StartX, int RowNum, int LayerNum);
  std::string *RLERowParent(std::string Row, int ParentX, int NumXBlocks);
  std::vector<std::vector<std::string>> GetMap() {return MapInformation;};
  char GetLetter(std::string Encoded, int Col);
};

#endif