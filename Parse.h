#ifndef PARSE_H
#define PARSE_H
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Helpers.h"
#include "Compression.h"
#include <cstring>
#include <chrono>
//#include <omp.h>


class Parse {
public:
  int XCount, YCount, ZCount;
  int ParentX, ParentY, ParentZ;
  int startX, startY, startZ;
  int Iterator;
  int NumXBlocks, NumYBlocks, NumZBlocks;
  int CacheHits = 0;
  int CacheMisses = 0;
  std::string TagTable[256];
  std::vector<std::string> Lines;
  std::vector<std::vector<std::string>> MapInformation;
  std::vector<ParentBlock> OutputBlocks;
public:
  Parse() {};
  Parse(const std::vector<std::string>& Lines);
  
  void StreamParseHeader(std::istream& infile);
  void StreamParseMapChunk(std::vector<ParentBlock>& ParentBlocks, int chunkIndex, std::istream& infile);

  int ParseHeader();
  void ParseMap();

  std::string TestRLERow(std::string Row);
  void RLERow(std::string_view BlockString, std::vector<Block>& RowBlocks, int StartX, int RowNum, int LayerNum);

  void Create3dKey(char * Key3d);
  void Create2dKey(std::string& Key2d, int localZ);
  void Create2dKey(char * Key2d, int localZ);
  bool UniformCheck(char * Key);
  bool UniformCheck(std::string& Key);

  std::vector<std::vector<std::string>> GetMap() {return MapInformation;};
  std::string * GetTagTable() {return TagTable;};
  void CollectOutput(std::vector<ParentBlock>& ParentBlocks);
  std::string TestCollectOutput(std::vector<ParentBlock>& ParentBlocks);
};

#endif