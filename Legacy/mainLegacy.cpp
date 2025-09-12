#include "Compression.h"
#include "Parse.h"
#include "Helpers.h"
#include "Test.h"
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;




int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    Args Args;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];
            if (arg == "-f") {
                if (i+1 < argc) {
                    Args.readFile = true;
                    Args.filePath = argv[++i];
                } else {
                    cerr << "Error: -f flag requires a file path argument." << endl;
                    return 1;
                }
                
            } else if (arg == "-t") {
                Args.TestingMode = true;
            } else if (arg == "-v") {
                if (i+1 < argc) {
                    if (isdigit(argv[i+1][0])){
                        Args.verboseLevel = stoi(argv[++i]);
                    }
                }
                Args.verbose = true;
            } else {
                cerr << "Unknown argument: " << arg << endl;
                return 1;
            }
        }
        if (Args.readFile) {
            cout << "Reading file: " << Args.filePath << endl;
            // Implement file reading logic here
        }
        if (Args.TestingMode) {
            cout << "Testing mode enabled." << endl;
            // Implement testing mode logic here
        }
        if (Args.verbose) {
            cout << "Verbose output enabled." << endl;
            // Implement verbose output logic here
        }
    }
    Parse Parser;
    Compression Compressor;

    std::string* AllMappings;

    std::vector<std::vector<std::vector<Block>>> XBlocks;
    std::ostringstream Output;
    vector<string> InitLines;
    string Line;
    if (Args.readFile) {
            ifstream infile("TestCases/"+Args.filePath);
            while (getline(infile, Line)) {
                if (!Line.empty() && Line.back() == '\r')
                {
                    Line.pop_back();
                }
                InitLines.push_back(Line);
            }
        infile.close();
        } else {
            while(std::getline(std::cin, Line)) {
                InitLines.push_back(Line);
            }
        }
    Parser = Parse(InitLines);
    Compressor = Compression();
    AllMappings = Parser.TagTable;
    XBlocks = Parser.XBlocks;
    auto TotalStart = chrono::high_resolution_clock::now();
    auto start = chrono::high_resolution_clock::now();
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    if (Args.TestingMode) {
        if (Args.verbose){
            elapsed = chrono::high_resolution_clock::now() - TotalStart;
            cout << "Parsing done in " << elapsed.count() << " seconds.\n";
            cout << "\n--- COMPRESSING --- \n";
        }
    }
    for (size_t z = 0; z < XBlocks.size(); ++z) {
            if (Args.verbose) { start = std::chrono::high_resolution_clock::now();}
        Compressor.ProcessLayer(XBlocks[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ, z, Output, AllMappings);
            if (Args.verbose) {
                end = std::chrono::high_resolution_clock::now();
                elapsed = end - start;
                std::cout << "Layer " << z << " processed in " << elapsed.count() << " seconds.\n";
            }
    }
    if (Parser.ParentZ != 1) {
            if (Args.verbose) {
                cout << "Merging layers now\n";
                start = std::chrono::high_resolution_clock::now();
            }
        Compressor.MergeLayers(Compressor.GetBlocks(), Parser.ParentZ);
            if (Args.verbose) {
                end = std::chrono::high_resolution_clock::now();
                elapsed = end - start;
                std::cout << "Merging Z axis done in " << elapsed.count() << " seconds.\n";
            }
       
    }
        if (Args.verbose) {
            cout << "Writing blocks now\n";
            start = std::chrono::high_resolution_clock::now();
        }
    Compressor.WriteBlocks(Compressor.GetBlocks(), Output, AllMappings);
        if (Args.verbose) {
            end = std::chrono::high_resolution_clock::now();
            elapsed = end - start;
            cout << "Writing blocks done in " << elapsed.count() << " seconds.\n";
        }
    
    if (Args.TestingMode){
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - TotalStart;
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
    Test myTest(InitLines, outputLines);

    std::cout << "--- COMPARE PARSE --- \n";
    if (Args.verbose && Args.verboseLevel > 1) { 
        myTest.printInputParse();
        myTest.printOutputParse();
    }
    

    bool match = true; //myTest.compareInputOutput();

    // --- Compute compression percentage ---
    size_t inputSize = 0;
    for (auto &layer : myTest.InputMapExpanded)
        for (auto &row : layer)
            inputSize += row.size();   // total number of characters in expanded input

    size_t compressedRows = outputLines.size(); // each rectangle is one line

    double compressionPercent = 100.0 * (1.0 - double(compressedRows) / double(inputSize));

    //myTest.printOutputBlocks();
    
    // --- Report results ---
    std::cout << "| Test Success | " << match << " || Compression Time | " << elapsed.count() << " seconds" << " || Compression % | " << compressionPercent << "% |\n";
    } else {
        std::cout << Output.str();
    }
    

    return 0;

}