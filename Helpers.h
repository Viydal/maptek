#include <string>

#ifndef HELPERS_H
#define HELPERS_H

using namespace std;

struct Args {
    bool readFile = false;
    std::string filePath = "";
    bool TestingMode = false;
    bool TestAll = false;
    bool verbose = false;
    int verboseLevel = 1;
};


struct Block {
  Block() : XPos(0), YPos(0), ZPos(0), XSize(0), YSize(0), ZSize(0), Ch('?') {}
  Block(int x, int y, int z, int xs, int ys, int zs, char ch) : XPos(x), YPos(y), ZPos(z), XSize(xs), YSize(ys), ZSize(zs), Ch(ch) {}
  int XPos, YPos, ZPos;
  int XSize, YSize, ZSize;
  char Ch;
};
#endif