#include "Parse.h"
#include "Compression.h"
#include "Test.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>  // for timing

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

    // --- Read input file ---
    std::ifstream infile(argv[1]);
    std::vector<std::string> Lines;
    std::string Line;
    while (std::getline(infile, Line)) Lines.push_back(Line);
    infile.close();

    // --- Prepare Parser & Compressor ---
    Parse Parser(Lines);
    Compression Compressor;
    std::ostringstream Output;
    auto Map = Parser.GetMap();
    auto AllMappings = Parser.GetTagTable();

    // --- Compression loop with timing ---
    std::cout << "Compressing...\n";
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t z = 0; z < Map.size(); ++z) {
        Compressor.ProcessLayer(Map[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ, z, Output, AllMappings);
        Output << "\n"; // ensure line separation
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Compression done in " << elapsed.count() << " seconds.\n";

    // --- Split compressed output into lines ---
    std::vector<std::string> outputLines;
    std::stringstream ss(Output.str());
    std::string outputLine;
    while (std::getline(ss, outputLine)) {
        if (!outputLine.empty())
            outputLines.push_back(outputLine);
    }

    // --- Create Test object and compare ---
    Test myTest(Lines, outputLines);

    myTest.printInputParse();
    myTest.printOutputParse();

    bool match = myTest.compareInputOutput();

    // --- Report results ---
    std::cout << "| Test Success | " << match << " || Time | " << elapsed.count() << " seconds |\n";
    return 0;
}
