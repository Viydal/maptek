#include "Test.h"
void Test::reconstructOutputParse() {
        // Reverse TagTable: map full label → single char
        std::unordered_map<std::string, char> reverseMap;
        for (auto &pair : InputParse.TagTable) {
            reverseMap[pair.second] = pair.first;
        }

        // Initialize expanded map for each layer
        std::vector<std::vector<std::string>> reconMap(
            InputParse.ZCount,
            std::vector<std::string>(InputParse.YCount, std::string(InputParse.XCount, ' '))
        );

        // Temporary expanded 3D map (layer by layer)
        std::vector<std::vector<std::string>> expandedMap(InputParse.ZCount,
            std::vector<std::string>(InputParse.YCount, std::string(InputParse.XCount, ' ')));

        for (auto &line : outputLines) {
            int x, y, z, w, h, d;
            std::string label;
            char comma;
            std::stringstream ss(line);
            ss >> x >> comma >> y >> comma >> z >> comma >> w >> comma >> h >> comma >> d >> comma;
            ss >> label;

            char codeChar = reverseMap[label];  // map full label back to single char

            // Fill rectangle in expanded map
            for (int yy = y; yy < y + h; ++yy) {
                for (int xx = x; xx < x + w; ++xx) {
                    expandedMap[z][yy][xx] = codeChar;
                }
            }
        }

        // Convert expanded rows back into RLE for OutputParse
        OutputParse.MapInformation.resize(InputParse.ZCount);
        for (size_t z = 0; z < InputParse.ZCount; ++z) {
            for (size_t y = 0; y < InputParse.YCount; ++y) {
                std::string rleRow = InputParse.RLERow(expandedMap[z][y]);
                OutputParse.MapInformation[z].push_back(rleRow);
            }
        }

        // Copy other fields
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

    auto inputMap = InputParse.GetMap();
    auto outputMap = OutputParse.GetMap();

    for (size_t layer = 0; layer < inputMap.size(); layer++) {
        for (size_t row = 0; row < inputMap[layer].size(); row++) {
            std::string inputRow = inputMap[layer][row];
            std::string outputRow = outputMap[layer][row];

            // Expand RLE for comparison
            std::string expandedInput;
            for (size_t i = 0; i < inputRow.length(); ) {
                int count = 0;
                while (i < inputRow.size() && isdigit(inputRow[i])) {
                    count = count * 10 + (inputRow[i] - '0');
                    i++;
                }
                char ch = inputRow[i++];
                expandedInput.append(count, ch);
            }

            std::string expandedOutput;
            for (size_t i = 0; i < outputRow.length(); ) {
                int count = 0;
                while (i < outputRow.size() && isdigit(outputRow[i])) {
                    count = count * 10 + (outputRow[i] - '0');
                    i++;
                }
                char ch = outputRow[i++];
                expandedOutput.append(count, ch);
            }

            // Compare character by character
            for (size_t col = 0; col < expandedInput.size(); col++) {
                if (col >= expandedOutput.size() || expandedInput[col] != expandedOutput[col]) {
                    std::cout << "Mismatch at layer " << layer << ", row " << row << ", col " << col << ": input='" << expandedInput[col] << "' output='" << (col < expandedOutput.size() ? expandedOutput[col] : '-') << "'\n";
                    allMatch = false;
                    break; // Stop at first mismatch in this row
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