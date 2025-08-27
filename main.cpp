#include "Parse.h"
#include "Compression.h"
#include <iostream>
#include <string>

int main() {
    // std::cout << "Enter input:" << std::endl;
    std::string line;
    std::vector<std::string> lines;

    while(std::getline(std::cin, line)) {
        lines.push_back(line);
    }

    Parse parser = Parse(lines);
    Compression compressor = Compression();

    // Print all integer values
    // std::cout << "\n=== INTEGER VALUES ===" << std::endl;
    // std::cout << "Xcount: " << parser.Xcount << std::endl;
    // std::cout << "Ycount: " << parser.Ycount << std::endl;
    // std::cout << "Zcount: " << parser.Zcount << std::endl;
    // std::cout << "ParentX: " << parser.ParentX << std::endl;
    // std::cout << "ParentY: " << parser.ParentY << std::endl;
    // std::cout << "ParentZ: " << parser.ParentZ << std::endl;
    
    // Print all TagTable entries
    // std::cout << "\n=== TAG TABLE (std::unordered_map<char, std::string>) ===" << std::endl;
    std::unordered_map<char, std::string> allMappings = parser.getTagTable();
    // for (const auto& pair : allMappings) {
    //     std::cout << "TagTable['" << pair.first << "'] = \"" << pair.second << "\"" << std::endl;
    // }

    std::vector<std::vector<std::string>> map = parser.GetMap();

    std::ostringstream output;

    for (size_t z = 0; z < map.size(); z++) {
        // for (size_t j = 0; j < map[i].size(); j++) {
        //     // std::cout << "Row " << j << " of layer " << i << ": " << map[i][j] << std::endl;
        //     std::cout << compressor.SingleLineCompress(map[i][j], allMappings, parser.ParentX, parser.ParentY, parser.ParentZ, j, i);
        // }
        // std::cout << std::endl;
        compressor.ProcessLayer(map[z], parser.ParentX, parser.ParentY, parser.ParentZ, z, output, allMappings);
        // compressor.Uncompress2d(compressor.ProcessLayer(map[i], parser.ParentX, parser.ParentY, parser.ParentZ, i, output, allMappings), allMappings, parser.Xcount, parser.Ycount);
    }

    std::cout << output.str();
}