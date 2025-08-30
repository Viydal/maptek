#include "Test.h"
void Test::reconstructOutputParse() {
    // Reverse TagTable: full label -> single char
    std::unordered_map<std::string, int> reverseMap;
    for (int i = 0; i<256; i++) {
        if (!InputParse.TagTable[i].empty()) {
            reverseMap[InputParse.TagTable[i]] = i;
        }
        
    }

    // Temporary expanded map
    OutputMapExpanded.resize(InputParse.ZCount, std::vector<std::string>(InputParse.YCount, std::string(InputParse.XCount, ' ')));

    for (auto &line : outputLines) {
        int x, y, z, w, h, d;
        std::string label;
        char comma;
        std::stringstream ss(line);
        ss >> x >> comma >> y >> comma >> z >> comma >> w >> comma >> h >> comma >> d >> comma;
        ss >> label;
        char codeChar = reverseMap[label];

        for (int yy = y; yy < y + h; ++yy) {
            for (int xx = x; xx < x + w; ++xx) {
                OutputMapExpanded[z][yy][xx] = codeChar;
            }
        }
    }

    // Convert expanded rows back into RLE for OutputParse
    OutputParse.MapInformation.resize(InputParse.ZCount);
    for (size_t z = 0; z < InputParse.ZCount; ++z) {
        for (size_t y = 0; y < InputParse.YCount; ++y) {
            std::string rleRow = InputParse.TestRLERow(OutputMapExpanded[z][y]);
            OutputParse.MapInformation[z].push_back(rleRow);
        }
    }

    // Copy other Parse fields
    OutputParse.XCount = InputParse.XCount;
    OutputParse.YCount = InputParse.YCount;
    OutputParse.ZCount = InputParse.ZCount;
    OutputParse.ParentX = InputParse.ParentX;
    OutputParse.ParentY = InputParse.ParentY;
    OutputParse.ParentZ = InputParse.ParentZ;
    OutputParse.NumXBlocks = InputParse.NumXBlocks;
    OutputParse.NumYBlocks = InputParse.NumYBlocks;
    OutputParse.NumZBlocks = InputParse.NumZBlocks;
}

// Compare InputParse and OutputParse maps
bool Test::compareInputOutput() {
    bool allMatch = true;

    // Use fully expanded maps stored in the class
    auto &inputMap = InputMapExpanded;   // InputParse fully expanded
    auto &outputMap = OutputMapExpanded; // OutputParse fully expanded

    for (size_t layer = 0; layer < inputMap.size(); ++layer) {
        for (size_t row = 0; row < inputMap[layer].size(); ++row) {
            for (size_t col = 0; col < inputMap[layer][row].size(); ++col) {
                char inChar = inputMap[layer][row][col];
                char outChar = (col < outputMap[layer][row].size()) ? outputMap[layer][row][col] : '-';
                if (inChar != outChar) {
                    std::cout << "Mismatch at layer " << layer
                              << ", row " << row
                              << ", col " << col
                              << ": input='" << inChar
                              << "' output='" << outChar << "'\n";
                    allMatch = false;
                    break; // stop at first mismatch in this row
                }
            }
        }
    }

    return allMatch;
}

// Print stored InputParse map
void Test::printInputParse() {
    std::cout << "=== InputParse Map ===\n";
    auto map = InputParse.GetMap();
    for (size_t z = 0; z < map.size(); ++z) {
        std::cout << "Layer " << z << ":\n";
        for (size_t y = 0; y < map[z].size(); ++y) {
            std::cout << map[z][y] << "\n";
        }
        std::cout << "\n";
    }
}

// Print stored OutputParse map
void Test::printOutputParse() {
    std::cout << "=== OutputParse Map ===\n";
    auto map = OutputParse.GetMap();
    for (size_t z = 0; z < map.size(); ++z) {
        std::cout << "Layer " << z << ":\n";
        for (size_t y = 0; y < map[z].size(); ++y) {
            std::cout << map[z][y] << "\n";
        }
        std::cout << "\n";
    }
}

void Test::printInputMapExpanded() {
    std::cout << "=== Expanded Input Map ===\n";
    for (size_t z = 0; z < InputMapExpanded.size(); ++z) {
        std::cout << "Layer " << z << ":\n";
        for (const auto &row : InputMapExpanded[z]) {
            std::cout << row << "\n";
        }
        std::cout << "\n";
    }
}

// Print fully expanded OutputMap
void Test::printOutputMapExpanded() {
    std::cout << "=== Expanded Output Map ===\n";
    for (size_t z = 0; z < OutputMapExpanded.size(); ++z) {
        std::cout << "Layer " << z << ":\n";
        for (const auto &row : OutputMapExpanded[z]) {
            std::cout << row << "\n";
        }
        std::cout << "\n";
    }
}

void Test::printOutputLines() {
    std::cout << "=== Output Lines ===\n";
    for (const auto &line : outputLines) {
        std::cout << line << "\n";
    }
    std::cout << "\n";
}

void Test::printOutputBlocks() {
    //TBD display blocks in map
}