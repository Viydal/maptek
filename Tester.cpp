#include "Tester.h"
#include "Compression.h"
#include "Parse.h"
#include "Test.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

bool Tester::RunTest(const std::string &filePath, bool verbose, int verboseLevel) {
    std::ifstream infile("TestCases/" + filePath);
    if (!infile) {
        std::cerr << "Error: could not open " << filePath << "\n";
        return false;
    }

    std::vector<std::string> InitLines;
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        InitLines.push_back(line);
    }
    infile.close();

    Parse Parser(InitLines);
    Compression Compressor;
    std::ostringstream Output;

    auto TotalStart = std::chrono::high_resolution_clock::now();

    size_t totalLayers = Parser.XBlocks.size();
    for (size_t z = 0; z < totalLayers; ++z) {
        Compressor.ProcessLayer(Parser.XBlocks[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ,
                                z, Output, Parser.TagTable);

        // Show per-test progress bar for verbose level 0 and above
        if (verbose && verboseLevel >= 0) {
            int barWidth = 50;
            int progress = static_cast<int>((z + 1) * barWidth / totalLayers);
            std::cout << "\r[";
            for (int i = 0; i < barWidth; ++i)
                std::cout << (i < progress ? '#' : ' ');
            std::cout << "] " << (z + 1) * 100 / totalLayers << "% completed" << std::flush;
        }
    }

    if (Parser.ParentZ != 1) {
        Compressor.MergeLayers(Compressor.GetBlocks(), Parser.ParentZ);
    }
    Compressor.WriteBlocks(Compressor.GetBlocks(), Output, Parser.TagTable);

    if (verbose && verboseLevel == 0)
        std::cout << "\n"; // move to new line after status bar

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - TotalStart;

    // Collect output lines
    std::vector<std::string> outputLines;
    std::stringstream ss(Output.str());
    while (std::getline(ss, line)) {
        if (!line.empty())
            outputLines.push_back(line);
    }

    // Compare input/output
    Test myTest(InitLines, outputLines);
    bool match = myTest.compareInputOutput();

    // Compute compression %
    size_t inputSize = 0;
    for (auto &layer : myTest.InputMapExpanded)
        for (auto &row : layer)
            inputSize += row.size();

    size_t compressedRows = outputLines.size();
    double compressionPercent = 100.0 * (1.0 - double(compressedRows) / double(inputSize));

    // Verbose level 1: show full per-test output
    if (verbose && verboseLevel >= 1) {
        std::cout << "\n--- TEST: " << filePath << " ---\n";
        myTest.printInputParse();
        myTest.printOutputParse();
        myTest.printOutputBlocks();

        std::cout << "| Test Success | " << match
                  << " || Time | " << elapsed.count() << "s"
                  << " || Compression % | " << compressionPercent << "% |\n";
    }

    return match;
}

void Tester::RunAllTests(bool verbose, int verboseLevel) {
    std::vector<std::string> testFiles;
    std::vector<std::string> failedFiles;

    // Collect all valid test filenames
    for (const auto &entry : fs::directory_iterator("TestCases")) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.size() >= 5 && filename[0] == 'T' &&
                filename[filename.size() - 4] == '.' &&
                filename.substr(filename.size() - 4) == ".txt" &&
                std::all_of(filename.begin() + 1, filename.end() - 4, ::isdigit))
            {
                testFiles.push_back(filename);
            }
        }
    }

    int total = static_cast<int>(testFiles.size());
    int passed = 0;

    for (size_t i = 0; i < testFiles.size(); ++i) {
        std::string filename = testFiles[i];
        std::string filepath = "TestCases/" + filename;

        // Verbose 0+: print which test is running
        if (verbose) {
            std::cout << "Running test file: " << filepath << " [" << i + 1 << "/" << total << "]" << std::endl;
        }

        bool ok = RunTest(filename, verbose, verboseLevel);
        if (ok) passed++;
        else failedFiles.push_back(filename);

        // Overall batch progress bar (optional for verbose 0)
        if (verbose && verboseLevel == 0) {
            int barWidth = 50;
            int progress = static_cast<int>((i + 1) * barWidth / total);
            std::cout << "[";
            for (int j = 0; j < barWidth; ++j)
                std::cout << (j < progress ? '#' : ' ');
            std::cout << "] " << (i + 1) * 100 / total << "% Overall\n\n";
        }
    }

    // Final summary
    std::cout << "\n--- SUMMARY ---\n";
    std::cout << "Total tests: " << total << "\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failedFiles.size() << "\n";

    if (!failedFiles.empty()) {
        std::cout << "Failed test files:\n";
        for (auto &f : failedFiles) {
            std::cout << "  - " << f << "\n";
        }
    }
}


