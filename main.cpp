#include "Compression.h"
#include "Parse.h"
#include <iostream>
#include <string>

int main() {

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

    // Go through each block 
    for (size_t z = 0; z < XBlocks.size(); z++) {
        Compressor.ProcessLayer(XBlocks[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ, z, Output, AllMappings);
    }
    // If the blocks can e greater than 1 layer in depth
    /*
    if (Parser.ParentZ != 1) {
        Compressor.MergeLayers(Compressor.GetBlocks(), Parser.ParentZ);
        Compressor.WriteBlocks(Compressor.GetFinalBlocks(), Output, AllMappings);
    } else { // Otherwise print the blocks
        Compressor.WriteBlocks(Compressor.GetBlocks(), Output, AllMappings);
    }
    */
    Compressor.WriteBlocks(Compressor.GetBlocks(), Output, AllMappings);
    std::cout << Output.str();
}