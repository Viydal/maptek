#include "Parse.h"
#include "Compression.h"
#include "MultiLineCompression.h"
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
    MultiLineCompression MultiLineCompressor = MultiLineCompression();
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
    std::vector<std::string> output;

    for (size_t i = 0; i < map.size(); i++) {
        for (size_t j = 0; j < map[i].size(); j++) {
            output.push_back(compressor.SingleLineCompress(map[i][j], allMappings, parser.ParentX, parser.ParentY, parser.ParentZ, j, i));
        }
    }

    // Multi-line compression
    std::vector<std::string> uncompress;
    uncompress.push_back(MultiLineCompressor.MultiLineCompress(output, allMappings, parser.Xcount, parser.Ycount, parser.ParentY));
    std::cout<<uncompress[0];

    /*Uncoment to decompress Multiline compression*/
    //std::cout<<std::endl<<"UNCOMPRESSION"<<std::endl<<std::endl;
    //compressor.Uncompress2d(uncompress, allMappings, parser.Xcount, parser.Ycount);

    /*Uncoment to decompress single line compression*/
    // uncompresses output, compare with input to ensure compression is correct;
    //compressor.Uncompress2d(output, allMappings, parser.Xcount, parser.Ycount);
    
}