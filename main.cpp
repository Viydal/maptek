#include "Compression.h"
#include "Parse.h"
#include <iostream>
#include <string>

int main() {

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string Line;
    std::vector<std::string> Lines;

    while(std::getline(std::cin, Line)) {
        Lines.push_back(Line);
    }

    Parse Parser = Parse(Lines);
    Compression Compressor = Compression();

    std::string* AllMappings = Parser.TagTable;

    std::vector<std::vector<std::vector<Block>>> XBlocks = Parser.XBlocks;
    std::ostringstream Output;

    for (size_t z = 0; z < XBlocks.size(); z++) {
        Compressor.ProcessLayer(XBlocks[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ, z, Output, AllMappings);
    }
    if (Parser.ParentZ != 1) {
        Compressor.MergeLayers(Compressor.GetBlocks(), Parser.ParentZ);
        Compressor.WriteBlocks(Compressor.GetFinalBlocks(), Output, AllMappings);
    } else {
        Compressor.WriteBlocks(Compressor.GetBlocks(), Output, AllMappings);
    }

    // for (size_t layer = 0; layer < XBlocks.size(); layer++) {
    //   for (size_t row = 0; row < XBlocks[0].size(); row++) {
    //     for (size_t block = 0; block < XBlocks[0][0].size(); block++) {
    //       std::cout << "X: " << XBlocks[layer][row][block].XPos << ", Y: " << XBlocks[layer][row][block].YPos << ", X-size: " << XBlocks[layer][row][block].XSize << ", z-layer: " << XBlocks[layer][row][block].ZPos << std::endl;
    //     }
    //   }
    // }

    std::cout << Output.str();
}