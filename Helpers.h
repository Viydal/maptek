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

    uint8_t XPos, XSize;
    uint8_t YPos, YSize;
    uint8_t ZPos, ZSize;
    char Ch;
    bool Merged = false;
};

#include <vector>
class ParentBlock{
public:
    std::vector<Block> Blocks;
    uint16_t StartX, StartY, StartZ;
    uint8_t LimitX, LimitY, LimitZ;

    bool IsUniform = true;
    bool UniformInit = false;
    char UniformChar = 0;

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
    
    
    // ParentBlock(const ParentBlock& other)
    //     : Blocks(other.Blocks),
    //       StartX(other.StartX), StartY(other.StartY), StartZ(other.StartZ),
    //       LimitX(other.LimitX), LimitY(other.LimitY), LimitZ(other.LimitZ),
    //       IsUniform(other.IsUniform){ 
    //     copyCounter++; 
    // }

    // ParentBlock& operator=(const ParentBlock& other) {
    //     if (this != &other) {
    //         Blocks = other.Blocks;
    //         StartX = other.StartX;   StartY = other.StartY;   StartZ = other.StartZ;
    //         LimitX = other.LimitX;   LimitY = other.LimitY;   LimitZ = other.LimitZ;
    //         IsUniform = other.IsUniform;
    //         copyCounter++;
    //     }
    //     return *this;
    // }

    // ParentBlock(ParentBlock&& other) noexcept
    //     : Blocks(std::move(other.Blocks)),
    //       StartX(other.StartX), StartY(other.StartY), StartZ(other.StartZ),
    //       LimitX(other.LimitX), LimitY(other.LimitY), LimitZ(other.LimitZ),
    //       IsUniform(other.IsUniform){
    //         moveCounter++; 
    //     }

    // ParentBlock& operator=(ParentBlock&& other) noexcept {
    //     if (this != &other) {
    //         Blocks    = std::move(other.Blocks);
    //         StartX    = other.StartX;   StartY = other.StartY;   StartZ = other.StartZ;
    //         LimitX    = other.LimitX;   LimitY = other.LimitY;   LimitZ = other.LimitZ;
    //         IsUniform = other.IsUniform;
    //         moveCounter++;
    //     }
    //     return *this;
    // }

    //--------------------------------------------------------------------------------------chat gpt code
    inline void append_u32(std::string& s, uint16_t v) {
        char buf[10]; int n = 0;
        do { buf[n++] = char('0' + (v % 10)); v /= 10; } while (v);
        s.append(std::reverse_iterator<char*>(buf + n), std::reverse_iterator<char*>(buf));
    }

    void WriteBlock(const std::string* TagTable, std::string& out) {
        
        for (const Block& B : Blocks) {
            if (IsUniform) {
                // one line: whole parent volume at StartX/Y/Z with LimitX/Y/Z
                append_u32(out, StartX); out += ',';
                append_u32(out, StartY); out += ',';
                append_u32(out, StartZ); out += ',';
                append_u32(out, LimitX); out += ',';
                append_u32(out, LimitY); out += ',';
                append_u32(out, LimitZ); out += ',';
                out += TagTable[B.Ch];
                out += '\n';
                return;
            }

            append_u32(out, B.XPos + StartX); out += ',';
            append_u32(out, B.YPos + StartY); out += ',';
            append_u32(out, B.ZPos + StartZ); out += ',';
            append_u32(out, B.XSize);         out += ',';
            append_u32(out, B.YSize);         out += ',';
            append_u32(out, B.ZSize);         out += ',';
            out += TagTable[(unsigned char)B.Ch];
            out += '\n';
        }
    }

    // Old output methods
    void WriteBlock(const std::string* TagTable){
        std::string out;
        out.reserve(Blocks.size() * 30);
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
        std::cout.write(out.c_str(), out.size());
    }

    std::string TestWriteBlock(const std::string* TagTable){
        std::string out;
        out.reserve(Blocks.size() * 30);
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
        return out;
    }
};
#endif