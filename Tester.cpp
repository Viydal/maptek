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

bool Tester::RunTest(Args args) {
    std::ifstream infile("TestCases/" + args.filePath);
    if (!infile) {
        std::cerr << "Error: could not open " << args.filePath << "\n";
        return false;
    }

    std::vector<std::string> InitLines;
    std::string line;
    auto TotalStart = chrono::high_resolution_clock::now();
    auto start = chrono::high_resolution_clock::now();
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    while (std::getline(infile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        InitLines.push_back(line);
    }
    infile.close();

    Parse Parser(InitLines);
    Compression Compressor;
    std::ostringstream Output;

    if (args.verbose){
            elapsed = chrono::high_resolution_clock::now() - TotalStart;
            cout << "Parsing done in " << elapsed.count() << " seconds.\n";
            cout << "\n--- COMPRESSING --- \n";
        }
    std::ostringstream LayerCompressionTimes;
    size_t totalLayers = Parser.XBlocks.size();
    for (size_t z = 0; z < totalLayers; ++z) {
            if (args.verbose) { start = std::chrono::high_resolution_clock::now();}
        Compressor.ProcessLayer(Parser.XBlocks[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ,
                                z, Output, Parser.TagTable);
            

        // Show per-test progress bar for verbose level 0 and above
        if (args.verbose && args.verboseLevel >= 0) {
            int barWidth = 50;
            int progress = static_cast<int>((z + 1) * barWidth / totalLayers);
            std::cout << "\r[";
            for (int i = 0; i < barWidth; ++i)
                std::cout << (i < progress ? '#' : ' ');
            std::cout << "] " << (z + 1) * 100 / totalLayers << "% completed" << std::flush;
        }
        if (args.verbose) {
                end = std::chrono::high_resolution_clock::now();
                elapsed = end - start;
                LayerCompressionTimes << "Layer " << z << " processed in " << elapsed.count() << " seconds.\n";
        }
    }
    std::cout << std::endl << LayerCompressionTimes.str();
    if (Parser.ParentZ != 1) {
            if (args.verbose) {
                cout << "Merging layers now\n";
                start = std::chrono::high_resolution_clock::now();
            }
        Compressor.MergeLayers(Compressor.GetBlocks(), Parser.ParentZ);
            if (args.verbose) {
                end = std::chrono::high_resolution_clock::now();
                elapsed = end - start;
                std::cout << "Merging Z axis done in " << elapsed.count() << " seconds.\n";
            }
    }
        if (args.verbose) {
            cout << "Writing blocks now\n";
            start = std::chrono::high_resolution_clock::now();
        }
    Compressor.WriteBlocks(Compressor.GetBlocks(), Output, Parser.TagTable);
        if (args.verbose) {
            end = std::chrono::high_resolution_clock::now();
            elapsed = end - start;
            cout << "Writing blocks done in " << elapsed.count() << " seconds.\n";
        }

    if (args.verbose && args.verboseLevel == 0)
        std::cout << "\n"; // move to new line after status bar

    end = std::chrono::high_resolution_clock::now();
    elapsed = end - TotalStart;
    std::cout << "Program done in " << elapsed.count() << " seconds.\n";
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
    if (args.verbose && args.verboseLevel >= 2) {
        std::cout << "\n--- TEST: " << args.filePath << " ---\n";
        
        myTest.printOutputLines();
        
        if(args.verbose && args.verboseLevel >= 3) {
            std::cout << "-----INPUT V OUTPUT----- \n";
            myTest.printInputParse();
            myTest.printOutputParse();
        }

        
    }
    std::cout << "| Test Success | " << match
                << " || Time | " << elapsed.count() << "s"
                << " || Compression % | " << compressionPercent << "% |\n";
    return match;
}

void Tester::RunAllTests(Args args) {
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
        std::cout << "----------------------------------------\n\n";
        std::string filename = testFiles[i];
        std::string filepath = "TestCases/" + filename;
        args.filePath = filename;
        // Verbose 0+: print which test is running
        if (args.verbose) {
            std::cout << "Running test file: " << filepath << " [" << i + 1 << "/" << total << "]" << std::endl;
        }

        bool ok = RunTest(args);
        if (ok) passed++;
        else failedFiles.push_back(filename);

        // Overall batch progress bar (optional for verbose 0)
        if (args.verbose && args.verboseLevel == 0) {
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


