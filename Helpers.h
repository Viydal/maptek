#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <iostream>
#include <charconv>

using namespace std;

struct Args {
    bool readFile = false;
    std::string filePath = "";
    bool TestingMode = false;
    bool TestAll = false;
    bool verbose = false;
    int verboseLevel = 1;
};


inline int copyCounter = 0;
inline int moveCounter = 0;


struct Block {
    Block() : XPos(0), YPos(0), ZPos(0), XSize(0), YSize(0), ZSize(0), Ch('?') {}
    Block(int x, int y, int z, int xs, int ys, int zs, char ch) : XPos(x), YPos(y), ZPos(z), XSize(xs), YSize(ys), ZSize(zs), Ch(ch) {}

    //  Block(const Block& o)
    //     : XPos(o.XPos),YPos(o.YPos),ZPos(o.ZPos),
    //       XSize(o.XSize),YSize(o.YSize),ZSize(o.ZSize),
    //       Ch(o.Ch),Merged(o.Merged)
    // {copyCounter++;}

    // Block& operator=(const Block& o) {
    //     if (this != &o) {
    //         XPos=o.XPos; YPos=o.YPos; ZPos=o.ZPos;
    //         XSize=o.XSize; YSize=o.YSize; ZSize=o.ZSize;
    //         Ch=o.Ch; Merged=o.Merged;
    //         copyCounter++;
    //     }
    //     return *this;
    // }

    // Block(Block&& o) noexcept
    //     : XPos(o.XPos),YPos(o.YPos),ZPos(o.ZPos),
    //       XSize(o.XSize),YSize(o.YSize),ZSize(o.ZSize),
    //       Ch(o.Ch),Merged(o.Merged)
    // { moveCounter++; }

    // Block& operator=(Block&& o) noexcept {
    //     if (this != &o) {
    //         XPos=o.XPos; YPos=o.YPos; ZPos=o.ZPos;
    //         XSize=o.XSize; YSize=o.YSize; ZSize=o.ZSize;
    //         Ch=o.Ch; Merged=o.Merged;
    //         moveCounter++;
    //     }
    //     return *this;
    // }

    int XPos, YPos, ZPos;
    int XSize, YSize, ZSize;
    char Ch;
    bool Merged = false;
    void printBlock() {std::cout << "(" << XPos << "," << YPos << "," << ZPos << ", " << XSize << "," << YSize << "," << ZSize << ") " << Ch <<std::endl;}
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
    ParentBlock(int startX, int startY, int startZ, int ParentX, int ParentY, int ParentZ, char Key) 
    : StartX(startX), StartY(startY), StartZ(startZ), LimitX(ParentX), LimitY(ParentY), LimitZ(ParentZ) { 
        this->Blocks.emplace_back(0, 0, 0, ParentX, ParentY, ParentZ, Key);
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
    


    // // Copy ctor
    // ParentBlock(const ParentBlock& other)
    //     : Blocks(other.Blocks),
    //       StartX(other.StartX), StartY(other.StartY), StartZ(other.StartZ),
    //       LimitX(other.LimitX), LimitY(other.LimitY), LimitZ(other.LimitZ),
    //       IsUniform(other.IsUniform)
    // { ++copyCounter; }

    // // Copy assign
    // ParentBlock& operator=(const ParentBlock& other) {
    //     if (this != &other) {
    //         Blocks    = other.Blocks;
    //         StartX    = other.StartX;   StartY = other.StartY;   StartZ = other.StartZ;
    //         LimitX    = other.LimitX;   LimitY = other.LimitY;   LimitZ = other.LimitZ;
    //         IsUniform = other.IsUniform;
    //         ++copyCounter;
    //     }
    //     return *this;
    // }

    // // Move ctor (noexcept! vector will prefer this)
    // ParentBlock(ParentBlock&& other) noexcept
    //     : Blocks(std::move(other.Blocks)),
    //       StartX(other.StartX), StartY(other.StartY), StartZ(other.StartZ),
    //       LimitX(other.LimitX), LimitY(other.LimitY), LimitZ(other.LimitZ),
    //       IsUniform(other.IsUniform)
    // { ++moveCounter; }

    // // Move assign (noexcept!)
    // ParentBlock& operator=(ParentBlock&& other) noexcept {
    //     if (this != &other) {
    //         Blocks    = std::move(other.Blocks);
    //         StartX    = other.StartX;   StartY = other.StartY;   StartZ = other.StartZ;
    //         LimitX    = other.LimitX;   LimitY = other.LimitY;   LimitZ = other.LimitZ;
    //         IsUniform = other.IsUniform;
    //         ++moveCounter;
    //     }
    //     return *this;
    // }
    
    void WriteBlock(const std::string* TagTable) const {
        std::string out;
        out.reserve(Blocks.size() * 80);
        for (int i = 0; i < Blocks.size(); i++) { 
            const Block& B = Blocks[i];
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
        }
        std::cout.write(out.data(), static_cast<std::streamsize>(out.size())); 
    }

    std::string TestWriteBlock(const std::string* TagTable){
        std::string out;
        Block B;
        for (int i = 0; i < Blocks.size(); i++) { 
            B = Blocks[i];
            std::to_string(B.XPos + StartX);
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
        }
        return out;
    }
};
#endif