#include "Parse.h"
#include "Compression.h"
#include <iostream>
#include <string>

int main() {
    // std::cout << "Enter input:" << std::endl;
    std::string Line;
    std::vector<std::string> Lines;

    while(std::getline(std::cin, Line)) {
        Lines.push_back(Line);
    }

    Parse Parser = Parse(Lines);
    Compression Compressor = Compression();

    // Print all integer values
    // std::cout << "\n=== INTEGER VALUES ===" << std::endl;
    // std::cout << "Xcount: " << Parser.Xcount << std::endl;
    // std::cout << "Ycount: " << Parser.Ycount << std::endl;
    // std::cout << "Zcount: " << Parser.Zcount << std::endl;
    // std::cout << "ParentX: " << Parser.ParentX << std::endl;
    // std::cout << "ParentY: " << Parser.ParentY << std::endl;
    // std::cout << "ParentZ: " << Parser.ParentZ << std::endl;
    
    // Print all TagTable entries
    // std::cout << "\n=== TAG TABLE (std::unordered_map<char, std::string>) ===" << std::endl;
    std::unordered_map<char, std::string> AllMappings = Parser.GetTagTable();
    // for (const auto& pair : allMappings) {
    //     std::cout << "TagTable['" << pair.first << "'] = \"" << pair.second << "\"" << std::endl;
    // }

    std::vector<std::vector<std::string>> Map = Parser.GetMap();

    std::ostringstream Output;

    for (size_t z = 0; z < Map.size(); z++) {
        // for (size_t j = 0; j < Map[i].size(); j++) {
        //     // std::cout << "Row " << j << " of layer " << i << ": " << Map[i][j] << std::endl;
        //     std::cout << Compressor.SingleLineCompress(Map[i][j], allMappings, Parser.ParentX, Parser.ParentY, Parser.ParentZ, j, i);
        // }
        // std::cout << std::endl;
        Compressor.ProcessLayer(Map[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ, z, Output, AllMappings);
        // Compressor.Uncompress2d(Compressor.ProcessLayer(Map[i], Parser.ParentX, Parser.ParentY, Parser.ParentZ, i, Output, allMappings), allMappings, Parser.Xcount, Parser.Ycount);
    }

    std::cout << Output.str();
}