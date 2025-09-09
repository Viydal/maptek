#ifndef PARSE_H
#define PARSE_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Helpers.h"
#include "Compression.h"
//#include <omp.h>

class Parse {
public:
  int XCount, YCount, ZCount, ParentX, ParentY, ParentZ;
  int startX, startY, startZ;
  int Iterator;
  int NumXBlocks, NumYBlocks, NumZBlocks;
  std::string TagTable[256];
  std::vector<std::string> Lines;
  std::vector<std::vector<std::string>> MapInformation;
  std::vector<ParentBlock> OutputBlocks;
public:
  Parse() {};
  Parse(const std::vector<std::string>& Lines);
  
  
  int ParseHeader();
  void ParseMap();

  std::string TestRLERow(std::string Row);
  void RLERow(char* XBlockString, std::vector<Block> *RowBlocks, std::unordered_map<std::string, std::vector<std::pair<int,char>>> *DP,int StartX, int RowNum, int LayerNum);

  void Create3dKey(std::string& Key3d);
  void Create2dKey(std::string& Key2d, int localZ);
  bool UniformCheck(std::string& Key);

  std::vector<std::vector<std::string>> GetMap() {return MapInformation;};
  std::string * GetTagTable() {return TagTable;};
  std::string CollectOutput(std::vector<ParentBlock> ParentBlocks);
};

#endif