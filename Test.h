#ifndef TEST_H
#define TEST_H

#include "Parse.h"
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>

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

public:
    // Constructor: takes the input lines and output lines
    Test(const std::vector<std::string> &inputLines, const std::vector<std::string> &compressedLines)
        : InputParse(inputLines), outputLines(compressedLines) 
    {
        // Expand input map for direct access
        auto map = InputParse.GetMap();
        InputMapExpanded.resize(map.size());
        for (size_t z = 0; z < map.size(); ++z) {
            for (const auto &rleRow : map[z]) {
                std::string expanded;
                for (size_t i = 0; i < rleRow.size(); ) {
                    int count = 0;
                    while (i < rleRow.size() && isdigit(rleRow[i])) {
                        count = count * 10 + (rleRow[i] - '0');
                        i++;
                    }
                    char ch = rleRow[i++];
                    expanded.append(count, ch);
                }
                InputMapExpanded[z].push_back(expanded);
            }
        }

        // Reconstruct output Parse and expanded map
        reconstructOutputParse();
    }

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
};

#endif
