#include "MultiLineCompression.h"
#include <cmath>
MultiLineCompression::MultiLineCompression() {}

std::string MultiLineCompression::MultiLineCompress(std::vector<std::string> output, std::unordered_map<char, std::string> TagTable,int Xcount, int Ycount, int ParentY) {
    std::string CurrNum = "";
    std::vector<std::string> CompressedBlock;
    std::vector<std::vector<std::string>> CompressedMap;
    for (int i = 0; i < output.size(); i++) {
        for (int j = 0; j < output[i].size(); j++) {
            if (output[i][j] == ',') {
                CompressedBlock.push_back(CurrNum);
                CurrNum = "";
                continue;
            }
            if (output[i][j] == '\n' || output[i][j] == '\r') {
                CompressedBlock.push_back(CurrNum);
                CompressedMap.push_back(CompressedBlock);
                CompressedBlock.clear();
                CurrNum = "";
                continue;
            }
            CurrNum += output[i][j];
        }
    }
    if (!CurrNum.empty() || !CompressedBlock.empty()) {
        if (!CurrNum.empty()) CompressedBlock.push_back(CurrNum);
        if (!CompressedBlock.empty()) CompressedMap.push_back(CompressedBlock);
    }

    // Merge vertically adjacent blocks
    int BlockNumber = 0;
    while (BlockNumber < CompressedMap.size()) {
        for (int i = BlockNumber + 1; i < CompressedMap.size(); i++) {
            if (CompressedMap[BlockNumber][6] != CompressedMap[i][6]) continue;
            if (CompressedMap[BlockNumber][0] != CompressedMap[i][0]) continue;
            if (CompressedMap[BlockNumber][3] != CompressedMap[i][3]) continue;
            if (stoi(CompressedMap[BlockNumber][1]) + stoi(CompressedMap[BlockNumber][4]) != stoi(CompressedMap[i][1])) continue;
            if (ceil((stoi(CompressedMap[BlockNumber][4]) + stoi(CompressedMap[BlockNumber][1])) / ParentY) !=
                ceil((stoi(CompressedMap[BlockNumber][4]) + stoi(CompressedMap[BlockNumber][1]) + stoi(CompressedMap[i][1])) / ParentY)) continue;

            CompressedMap[BlockNumber][4] = std::to_string(stoi(CompressedMap[BlockNumber][4]) + stoi(CompressedMap[i][4]));
            CompressedMap.erase(CompressedMap.begin() + i);
            i--; 
        }
        BlockNumber++;
    }


    // Serialize CompressedMap to string

    std::ostringstream oss;
    for (const auto& block : CompressedMap) {
        if (block.size() < 7) continue;
        oss << block[0] << "," << block[1] << "," << block[2] << ","
            << block[3] << "," << block[4] << "," << block[5] << ","
            << block[6] << "\n";
        
    }
    return oss.str();
}