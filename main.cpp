#include "Tester.h"
#include "Compression.h"
#include "Parse.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

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

    auto start = std::chrono::high_resolution_clock::now();

    // Compression logic
    std::thread Thread1, Thread2, Thread3, Thread4;

    Parse Parser = Parse(InitLines);

    std::vector<ParentBlock> ParentBlocks = Parser.OutputBlocks;
    
    size_t TotalBlocks = ParentBlocks.size();
    size_t Quarter = TotalBlocks / 4;
    size_t Remainder = TotalBlocks % 4;

    // Find even splits for threads
    size_t Split1 = Quarter + (Remainder > 0 ? 1 : 0);
    size_t Split2 = Split1 + Quarter + (Remainder > 1 ? 1 : 0);
    size_t Split3 = Split2 + Quarter + (Remainder > 2 ? 1 : 0);

    Compression Compressor = Compression();
    std::string* AllMappings = Parser.TagTable;

    Thread1 = std::thread([&]() {
        Compression LocalCompressor;
        for (size_t i = 0; i < Split1; i++) {
            LocalCompressor.CompressParentBlock(ParentBlocks[i]);
        }
    });

    Thread2 = std::thread([&]() {
        Compression LocalCompressor;
        for (size_t i = Split1; i < Split2; i++) {
            LocalCompressor.CompressParentBlock(ParentBlocks[i]);
        }
    });

    Thread3 = std::thread([&]() {
        Compression LocalCompressor;
        for (size_t i = Split2; i < Split3; i++) {
            LocalCompressor.CompressParentBlock(ParentBlocks[i]);
        }
    });

    Thread4 = std::thread([&]() {
        Compression LocalCompressor;
        for (size_t i = Split3; i < TotalBlocks; i++) {
            LocalCompressor.CompressParentBlock(ParentBlocks[i]);
        }
    });

    Thread1.join();
    Thread2.join();
    Thread3.join();
    Thread4.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    if (Args.verbose) {
        std::cout << "Compression done in " << elapsed.count() << " seconds.\n";
    }

    std::cout << Parser.CollectOutput(ParentBlocks);

}

