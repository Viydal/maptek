#include "Compression.h"
#include "Parse.h"
#include <iostream>
#include <string>

void DrawMap(const std::vector<std::string> &compressedInput, int Xcount,
             int Ycount) {
  // Initialize grid filled with '.'
  std::vector<std::string> grid(Ycount, std::string(Xcount, '.'));

  // Label-to-char mapping
  std::unordered_map<std::string, char> labelToChar = {
      {"sea", 'o'}, {"WA", 'w'},  {"NT", 'n'},  {"SA", 's'},
      {"QLD", 'q'}, {"NSW", 'e'}, {"VIC", 'v'}, {"TAS", 't'}};

  for (const auto &line : compressedInput) {
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, ',')) {
      tokens.push_back(token);
    }

    if (tokens.size() < 7)
      continue;

    int XStart = std::stoi(tokens[0]);
    int YStart = std::stoi(tokens[1]);
    int XSize = std::stoi(tokens[3]);
    int YSize = std::stoi(tokens[4]);
    std::string label = tokens[6];

    char fillChar = (labelToChar.count(label) ? labelToChar[label] : '?');

    for (int y = YStart; y < YStart + YSize && y < Ycount; y++) {
      for (int x = XStart; x < XStart + XSize && x < Xcount; x++) {
        grid[y][x] = fillChar;
      }
    }
  }

  // Print from top (Ycount-1) down to 0
  for (int y = Ycount - 1; y >= 0; y--) {
    std::cout << grid[y] << "\n";
  }
}

int main() {
    std::string Line;
    std::vector<std::string> Lines;

    while(std::getline(std::cin, Line)) {
        Lines.push_back(Line);
    }

    Parse Parser = Parse(Lines);
    Compression Compressor = Compression();

    std::unordered_map<char, std::string> AllMappings = Parser.GetTagTable();

    std::vector<std::vector<std::string>> Map = Parser.GetMap();
    std::vector<std::vector<std::vector<Block>>> XBlocks = Parser.XBlocks;
    std::ostringstream Output;

    

    for (size_t z = 0; z < XBlocks.size(); z++) {
        Compressor.ProcessLayer(XBlocks[z], Parser.ParentX, Parser.ParentY, Parser.ParentZ, z, Output, AllMappings);
    }

    std::cout << Output.str();
}