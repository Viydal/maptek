#include "Tester.h"
#include "Compression.h"
#include "Parse.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>



int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Args Args;
    // --- Parse args ---
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f" && i + 1 < argc) {
            Args.readFile = true;
            Args.filePath = argv[++i];
        } else if (arg == "-t" || arg == "-ta") {
            Args.TestingMode = true;
            Args.TestAll = (arg == "-ta");
        } else if (arg == "-v"){
            if(i + 1 < argc && isdigit(argv[i + 1][0])) {
                Args.verboseLevel = std::stoi(argv[++i]);
                Args.verbose = true;
            } else {
                Args.verbose = true;
                Args.verboseLevel = 1;
            }
        } else {
            std::cerr << "Unknown or invalid argument: " << arg << "\n";
            return 1;
        }
    }
    if (Args.TestingMode) {
        cout << "Testing mode enabled." << endl;
        if (Args.TestAll) {
            std::cout << "Running all tests in TestCases/ directory.\n";
        }
    // Implement testing mode logic here
    }
    if (Args.readFile) {
        cout << "Reading file: " << Args.filePath << endl;
    // Implement file reading logic here
    }
    if (Args.verbose) {
        cout << "Verbose output enabled." << endl;
    // Implement verbose output logic here
    }
    // --- TESTING MODE ---
    if (Args.TestingMode) {
        if (Args.TestAll) {
            Tester::RunAllTests(Args);
        } else {
            if (!Args.readFile) {
                std::cerr << "Error: -t requires a file (-f <file>) or use -ta\n";
                return 1;
            }
            Tester::RunTest(Args);
        }
        return 0;
    }

    // --- NORMAL MODE ---
    std::vector<std::string> InitLines;
    std::string line;

    if (Args.readFile) {
        std::ifstream infile("TestCases/" + Args.filePath);
        if (!infile) {
            std::cerr << "Error: could not open " << Args.filePath << "\n";
            return 1;
        }
        while (std::getline(infile, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            InitLines.push_back(line);
        }
        infile.close();
    } else {
        // No flags: read from stdin
        while (std::getline(std::cin, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            InitLines.push_back(line);
        }
    }

    // --- Compression pipeline ---
    Parse Parser(InitLines);
    Compression Compressor;
    std::ostringstream Output;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t z = 0; z < Parser.XBlocks.size(); ++z) {
        Compressor.ProcessLayer(Parser.XBlocks[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ,
                                z, Output, Parser.TagTable);
    }

    if (Parser.ParentZ != 1) {
        Compressor.MergeLayers(Compressor.GetBlocks(), Parser.ParentZ);
    }

    Compressor.WriteBlocks(Compressor.GetBlocks(), Output, Parser.TagTable);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    if (Args.verbose) {
        std::cout << "Compression done in " << elapsed.count() << " seconds.\n";
    }

    // Print compressed output
    std::cout << Output.str();

    return 0;
}
