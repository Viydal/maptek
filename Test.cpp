#include "Test.h"
void Test::reconstructOutputParse() {
    // Reverse TagTable: full label -> single char
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
        //std::cout << "Line: " << line << " test- x:" << x << " y:" << y << " z:" << z << " w:" << w << " h:" << h << " d:" << d << std::endl;
        for (int zz = z; zz < z + d; ++zz) {
            for (int yy = y; yy < y + h; ++yy) {
                for (int xx = x; xx < x + w; ++xx) {
                    if(OutputMapExpanded[zz][yy][xx] != ' ') {
                        std::cout << "Error: overlapping blocks at (" << xx << "," << yy << "," << zz << ")" << std::endl;
                        std::cout << line << std::endl << OutputMapExpanded[zz][yy][xx] << std::endl;
                    }
                    OutputMapExpanded[zz][yy][xx] = codeChar;
                }
            }
        }
    }
    // Convert expanded rows back into RLE for OutputParse
    OutputParse.MapInformation.resize(InputParse.ZCount);
    for (size_t z = 0; z < InputParse.ZCount; ++z) {
        for (size_t y = 0; y < InputParse.YCount; ++y) {
            std::string rleRow = InputParse.TestRLERow(OutputMapExpanded[z][y]);
            //std::cout << "RLERow: " << rleRow << "  OME " << OutputMapExpanded[z][y] << std::endl;
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

void Test::MakeTest() {
// Example dimension line: "16, 6, 2, 4, 3, 2"
    std::string dims = outputLines[0];
    std::stringstream ss(dims);
    char comma;
    int x, y, z, X, Y, Z;
    ss >> x >> comma >> y >> comma >> z >> comma >> X >> comma >> Y >> comma >> Z;

    if (x*y*z > 10000000) {
        std::cout << "Fuck off im not doing that \n";
        exit(0);
    }
    
    // Fixed labels
    std::vector<std::pair<char, std::string>> labels = {
        {'o', "sea"},
        {'w', "WA"},
        {'n', "NT"},
        {'s', "SA"},
        {'q', "QLD"},
        {'e', "NSW"},
        {'v', "VIC"},
        {'t', "TAS"}
    };

    // Build test case lines
    std::vector<std::string> newLines;
    newLines.push_back(dims);
    for (auto &p : labels) {
        newLines.push_back(std::string(1, p.first) + ", " + p.second);
    }
    newLines.push_back(""); //blank line before map

    // Random generator
    std::mt19937 rng((unsigned)std::time(nullptr));
    std::uniform_int_distribution<int> labelDist(0, labels.size() - 1);
    std::uniform_int_distribution<int> percentDist(1, 7); // 1-7% chunk size

    int totalCells = x * y * z;
    int remaining = totalCells;


    //changed logic to generate in chunks rather than each individually

    // Flatten 3D map into 1D vector first
    std::vector<char> flatMap;
    while (remaining > 0) {
        int chunkPercent = percentDist(rng);
        int chunkSize = std::max(1, (chunkPercent * totalCells) / 100); // at least 1
        if (chunkSize > remaining) chunkSize = remaining;

        char label = labels[labelDist(rng)].first;
        flatMap.insert(flatMap.end(), chunkSize, label);
        remaining -= chunkSize;
    }

    // Shuffle the chunks so they are not in order
    std::shuffle(flatMap.begin(), flatMap.end(), rng);

    // Fill 3D map
    int index = 0;
    for (int zz = 0; zz < z; ++zz) {
        for (int yy = 0; yy < y; ++yy) {
            std::string row;
            for (int xx = 0; xx < x; ++xx) {
                row.push_back(flatMap[index++]);
            }
            newLines.push_back(row);
        }
        if (zz < z - 1) newLines.push_back(""); // blank line between z planes
    }

    //save info to test case
    testCase = newLines;

    // feed into input parse / Input map expanded
    InputParse = Parse(newLines);
    InputMapExpanded.resize(z, std::vector<std::string>(y, std::string(x, ' ')));

    int lineIndex = labels.size() + 2; //skip dims & labels
    for (int zz = 0; zz < z; ++zz) {
        for (int yy = 0; yy < y; ++yy) {
            InputMapExpanded[zz][yy] = newLines[lineIndex++];
        }
        lineIndex++; // skip blank line between slices
    }

    // Fill MapInformation (RLE compression)
    InputParse.MapInformation.resize(z);
    for (size_t zz = 0; zz < InputParse.ZCount; ++zz) {
        for (size_t yy = 0; yy < InputParse.YCount; ++yy) {
            std::string rleRow = InputParse.TestRLERow(InputMapExpanded[zz][yy]);
            InputParse.MapInformation[zz].push_back(rleRow);
        }
    }
}

Test::Test(const std::string &dimsLine)
    : InputParse(std::vector<std::string>{dimsLine}), outputLines({dimsLine}) {
    // At this stage we only know dimensions, not map content.
    // The test case will be generated by MakeTest()
}

void Test::saveTestCase() {
    namespace fs = std::filesystem;

    // Ensure folder exists
    fs::path folder("TestCases");
    if (!fs::exists(folder)) {
        fs::create_directory(folder);
    }

    // Find next available number
    int nextNumber = 1;
    while (true) {
        std::stringstream ss;
        ss << "T" << nextNumber << ".txt";
        fs::path filePath = folder / ss.str();
        if (!fs::exists(filePath)) {
            break;
        }
        nextNumber++;
    }

    // Create file path
    std::stringstream filename;
    filename << "T" << nextNumber << ".txt";
    fs::path filePath = folder / filename.str();

    // Write testCase to file
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        std::cerr << "Error: Cannot open file " << filePath << " for writing.\n";
        return;
    }

    for (const auto &line : testCase) {
        outFile << line << "\n";
    }

    outFile.close();
    std::cout << "Saved test case to " << filePath << "\n";
}
