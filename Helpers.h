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
    bool Merged = false;
};

#include <vector>
class ParentBlock{
public:
    std::vector<Block> Blocks;
    int StartX;
    int StartY; 
    int StartZ; 
    int LimitX; 
    int LimitY; 
    int LimitZ;
    bool IsUniform;
    ParentBlock(){};
    ParentBlock(std::vector<Block> Blocks) { this->Blocks = Blocks;}
    ParentBlock(int startX, int startY, int startZ, int ParentX, int ParentY, int ParentZ, char Key) 
    : StartX(startX), StartY(startY), StartZ(startZ), LimitX(ParentX), LimitY(ParentY), LimitZ(ParentZ) { 
        this->Blocks.push_back(Block(0, 0, 0, ParentX, ParentY, ParentZ, Key));
        this->IsUniform = true;
    }
    ParentBlock(int startX, int startY, int startZ, int ParentX, int ParentY, int ParentZ) 
    : StartX(startX), StartY(startY), StartZ(startZ), LimitX(ParentX), LimitY(ParentY), LimitZ(ParentZ) 
    {
        Blocks.reserve(ParentX * ParentY * ParentZ);
    }
    //ParentBlock(Block block) {
    //    this->Blocks.push_back(block);
    //}
    
    std::string WriteBlock(const std::string* TagTable){
        std::string out;
        Block B;
        for (int i = 0; i < Blocks.size(); i++) { 
            if (Blocks[i].Merged) continue;
            B = Blocks[i];
            out += std::to_string(B.XPos + StartX);
            out += ',';
            out += std::to_string(B.YPos  + StartY);
            out += ',';
            out += std::to_string(B.ZPos  + StartZ);
            out += ',';
            out += std::to_string(B.XSize);
            out += ',';
            out += std::to_string(B.YSize);
            out += ',';
            out += std::to_string(B.ZSize);
            out += ',';
            out += TagTable[B.Ch];
            out += '\n';
            //FormatOutput(Output, B.XPos, B.YPos, B.ZPos, B.XSize, B.YSize, B.ZSize, B.Ch, TagTable);
        }
        return out;
    }
};
#endif