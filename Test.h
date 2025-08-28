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

public:
    // Constructor: takes the input lines and output lines
    Test(const std::vector<std::string> &inputLines, const std::vector<std::string> &compressedLines) 
        : InputParse(inputLines), outputLines(compressedLines) 
    {
        reconstructOutputParse();
    }
    
    // Convert output lines (rectangles) into a Parse object
    void reconstructOutputParse();

    // Compare InputParse vs OutputParse and report all mismatches
    bool compareInputOutput();

    void printOutputParse();

    void printInputParse();
};

#endif
