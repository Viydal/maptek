#include "Parse.h"
#include <sstream>
#include <fstream>
#include <iostream>

Parse::Parse(const std::vector<std::string>& Lines) {
    //this->Lines = Lines;
    //ParseHeader();
    //ParseMap();

}

void Parse::StreamParseHeader(std::istream& in){
    char Delimeter;
    std::string Token;
    std::string Line;
    
    // first line is dimensions of input
    std::getline(in, Line);
    if(!Line.empty() && Line.back() == '\r') Line.pop_back();

    std::istringstream Ss(Line);
    Ss >> XCount >> Delimeter >> YCount >> Delimeter >> ZCount >> Delimeter
        >> ParentX >> Delimeter >> ParentY >> Delimeter >> ParentZ;
    NumXBlocks = XCount / ParentX;
    NumYBlocks = YCount / ParentY;
    NumZBlocks = ZCount / ParentZ;

    // the next lines are the tag table
    while (std::getline(in, Line)) {
        if (!Line.empty() && Line.back() == '\r') Line.pop_back();
        if (Line.empty()) {break;}

        char Symbol;
        std::string Location;
        std::istringstream Ss(Line);

        Ss >> Symbol >> Delimeter >> Location;
        TagTable[Symbol] = Location;
    }
}

void Parse::StreamParseMapChunk(std::vector<ParentBlock>& ParentBlocks, int chunkIndex, std::istream& in, std::unordered_map<std::string, std::vector<std::pair<int,char>>>& RleCache) {
    std::string Line;
    Line.reserve(XCount + 3);
    //
     for (int Z = 0; Z < ParentZ; Z++) {
        int RowsRead = 0;



        while (RowsRead < YCount && std::getline(in, Line)) {
            if (!Line.empty() && Line.back() == '\r') Line.pop_back();
            if (Line.empty()) continue;

            int StartY = RowsRead / ParentY;
            int LocalY = RowsRead % ParentY;

            int StartX = 0;
            for (int XBlockIndex = 0; XBlockIndex < NumXBlocks; XBlockIndex++) {
                int ParentBlockIndex = StartY * NumXBlocks + XBlockIndex;

                std::string_view Substring(Line.data() + StartX, ParentX);
                RLERow(Substring, ParentBlocks[ParentBlockIndex].Blocks, 0, LocalY, Z, RleCache);
                StartX += ParentX;
            }
            RowsRead++;
        }
    }
}


std::string Parse::TestCollectOutput(std::vector<ParentBlock>& ParentBlocks) {
    std::string Output;
    for (ParentBlock &PB : ParentBlocks){
        Output += PB.TestWriteBlock(TagTable);
    }
    return Output;
}

void Parse::Create3dKey(char* key3d) {
    size_t writePos = 0;
    const size_t stridePerLayer = YCount + 1;
    const size_t total = static_cast<size_t>(ParentX) * ParentY * ParentZ;

    for (int z = 0; z < ParentZ; ++z) {
        const int base = Iterator + (startZ + z) * static_cast<int>(stridePerLayer);
        for (int y = 0; y < ParentY; ++y) {
            const int lineIdx = base + (startY + y);
            const std::string& line = Lines[lineIdx];

            std::memcpy(key3d + writePos, line.data() + startX, ParentX);
            writePos += ParentX;
        }
    }
}

void Parse::Create2dKey(char * Key2d, int localZ){
    size_t writePos = 0;
    const size_t base = Iterator + (startZ + localZ) * (YCount + 1);
        for (int y = 0; y < ParentY; ++y) {
            const int lineIdx = base + (startY + y);
            const std::string& line = Lines[lineIdx];

            std::memcpy(Key2d + writePos, line.data() + startX, ParentX);
            writePos += ParentX;
        }
}

bool Parse::UniformCheck(char * Key){
    char first = Key[0];
    for (int i = 1; i < ParentX * ParentY * ParentZ; i++){
        if (Key[i] != first)
        return false;
    }
    return true;
}

void Parse::Create2dKey(std::string& Key2d, int localZ){
    Key2d.reserve(ParentX * ParentY);
    for (int y = 0; y < ParentY; y++) {
        int lineIdx =  Iterator + ((startZ + localZ) * (YCount + 1)) + startY + y;
        Key2d.append(Lines[lineIdx], startX, ParentX);
    }
}

bool Parse::UniformCheck(std::string& Key){
    char first = Key[0];
    for (int i = 1; i < Key.size(); i++){
        if (Key[i] != first)
        return false;
    }
    return true;
}

std::string Parse::TestRLERow(std::string Row) {
    std::string RLEString;
    int Counter = 0;
    char PrevChar = Row[0];

    for (size_t i = 0; i < Row.length(); i++) {
        char CurrChar = Row[i];
        if (CurrChar == PrevChar) {
            Counter++;
        } else {
            RLEString += std::to_string(Counter) + PrevChar;
            PrevChar = CurrChar;
            Counter = 1;
        }
    }

    RLEString += std::to_string(Counter) + PrevChar;
    return RLEString;
}

void Parse::RLERow(std::string_view BlockString, std::vector<Block>& RowBlocks, int StartX, int RowNum, int LayerNum, std::unordered_map<std::string, std::vector<std::pair<int,char>>>&){
 
    int Count = 1;
    char Prev = BlockString[0];
    char Current; 
    for (int i = 1; i < ParentX; i++){
        Current = BlockString[i];
        if (Current == Prev) {
            Count++;
        } else {
            RowBlocks.emplace_back(StartX, RowNum, LayerNum, Count, 1, 1, Prev);
            StartX += Count;
            Prev = Current;
            Count = 1;
        }
    }
    RowBlocks.emplace_back(StartX, RowNum, LayerNum, Count, 1, 1, Prev);
}