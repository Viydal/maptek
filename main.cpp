#include "Parse.h"
#include "Compression.h"
#include <iostream>
#include <string>

int main() {
    std::string Line;
    std::vector<std::string> Lines;

    while(std::getline(std::cin, Line)) {
        Lines.push_back(Line);
    }

    Parse Parser = Parse(Lines);
    Compression Compressor = Compression();

    std::unordered_map<char, std::string> AllMappings = Parser.GetTagTable();
    std::vector<std::vector<std::string>> Map = Parser.GetMap();

    std::string Output;
    for (size_t z = 0; z < Map.size(); z++) {
        Output += Compressor.ProcessLayer(Map[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ, z, AllMappings);
    }

    std::cout << Output;
}