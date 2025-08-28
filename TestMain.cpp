#include "Parse.h"
#include "Compression.h"
#include <iostream>
#include <string>
#include <fstream>   // for file input
#include <chrono> // for timing

int main(int argc, char* argv[]) {
    // Check if user provided a file path
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.txt>" << std::endl;
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::string Line;
    std::vector<std::string> Lines;

    // Read file line by line into Lines
    while (std::getline(infile, Line)) {
        Lines.push_back(Line);
    }

    infile.close();

    // Parse & Compress
    Parse Parser = Parse(Lines);
    Compression Compressor = Compression();

    std::unordered_map<char, std::string> AllMappings = Parser.GetTagTable();
    std::vector<std::vector<std::string>> Map = Parser.GetMap();

    std::ostringstream Output;

    std::cout << "Compressing..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t z = 0; z < Map.size(); z++) {
        Compressor.ProcessLayer(Map[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ, z, Output, AllMappings);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << Output.str();
    std::cout << "Done" << std::endl;

    std::cout << "Time taken for compression loop: " << elapsed.count() << " seconds" << std::endl;
}