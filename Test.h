#ifndef TEST_H
#define TEST_H

#include "Parse.h"
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <random>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <algorithm>

class Test {
public:
    // Original parsed input
    Parse InputParse;

    // Reconstructed output stored as a Parse object
    Parse OutputParse;

    std::vector<std::string> outputLines;

     // Fully expanded vector forms (not RLE)
    std::vector<std::vector<std::string>> InputMapExpanded;
    std::vector<std::vector<std::string>> OutputMapExpanded;
    std::unordered_map<std::string, char> reverseMap;

    //save a full generated test case:
    std::vector<std::string> testCase;

public:
    // Constructor: takes the input lines and output lines
    Test(const std::vector<std::string> &inputLines, const std::vector<std::string> &compressedLines, Parse& parser)
        : outputLines(compressedLines), InputParse(parser)
    {
        // Set InputParse dimensions to account for single chunk
        InputParse.NumZBlocks = 1;
        InputParse.ZCount = InputParse.ParentZ;
        
        // Expand input map for direct access
        InputMapExpanded.resize(InputParse.ZCount, std::vector<std::string>(InputParse.YCount, std::string(InputParse.XCount, ' ')));
        
        int Iterator = 0;
        while (inputLines[Iterator].empty()) {
            Iterator++;
        }
        int z_count = 0;
        int y_count = 0;
        while(Iterator < inputLines.size()){
            std::string line = inputLines[Iterator];
            if (inputLines[Iterator].empty()){
                z_count++;
                Iterator++;
                y_count = 0;
                continue;
            }
            for (int x = 0; x < line.size(); x++) {
                InputMapExpanded[z_count][(y_count) % InputParse.YCount][x] = (line[x]);
            }
            Iterator++;
            y_count++;
        }
        // Convert expanded rows back into RLE for OutputParse
        InputParse.MapInformation.resize(InputParse.ZCount);
        for (size_t z = 0; z < InputParse.ZCount; ++z) {
            for (size_t y = 0; y < InputParse.YCount; ++y) {
                std::string rleRow = InputParse.TestRLERow(InputMapExpanded[z][y]);
                InputParse.MapInformation[z].push_back(rleRow);
            }
        }
        // Reconstruct output Parse and expanded map
        reconstructOutputParse();
    }

    // Constructor for generation-only
    Test(const std::string &dimsLine);

    // Convert output lines (rectangles) into a Parse object
    void reconstructOutputParse();

    // Compare InputParse vs OutputParse and report all mismatches
    bool compareInputOutput();

    void printOutputParse();

    void printInputParse();

    void printInputMapExpanded();

    void printOutputMapExpanded();

    void printOutputLines();

    void printOutputBlocks();

    void MakeTest();

    void saveTestCase();
};

#endif