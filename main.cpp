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
  // std::cout << "Enter input:" << std::endl;
  std::string line;
  std::vector<std::string> lines;

  while (std::getline(std::cin, line)) {
    lines.push_back(line);
  }

  Parse parser = Parse(lines);
  Compression compressor = Compression();

  // Print all integer values
  // std::cout << "\n=== INTEGER VALUES ===" << std::endl;
  // std::cout << "Xcount: " << parser.Xcount << std::endl;
  // std::cout << "Ycount: " << parser.Ycount << std::endl;
  // std::cout << "Zcount: " << parser.Zcount << std::endl;
  // std::cout << "ParentX: " << parser.ParentX << std::endl;
  // std::cout << "ParentY: " << parser.ParentY << std::endl;
  // std::cout << "ParentZ: " << parser.ParentZ << std::endl;

  // Print all TagTable entries
  // std::cout << "\n=== TAG TABLE (std::unordered_map<char, std::string>) ==="
  // << std::endl;
  std::unordered_map<char, std::string> allMappings = parser.getTagTable();
  // for (const auto& pair : allMappings) {
  //     std::cout << "TagTable['" << pair.first << "'] = \"" << pair.second <<
  //     "\"" << std::endl;
  // }

  std::vector<std::vector<std::string>> map = parser.GetMap();

  // for (size_t i = 0; i < map.size(); i++) {
  //   for (size_t j = 0; j < map[i].size(); j++) {
  //     std::cout << map[i][j] << std::endl;
  //   }
  //   std::cout << std::endl;
  //   // std::vector<std::string> blocks = compressor.ExtractRegion(map[i]);
  // }

  std::vector<std::vector<std::vector<std::string>>> ParentBlockInformation =
      parser.ParentBlockInformation;
  std::vector<std::vector<std::string>> Blocks = parser.Blocks;

//   std::cout << parser.NumXBlocks << " " << parser.NumYBlocks << " " << parser.NumZBlocks << std::endl;

  for (size_t block = 0; block < ParentBlockInformation.size(); block++) {
    // std::cout << "Block " << block << ":\n";
    // std::cout << "--------\n";

    std::vector<std::vector<std::string>> IndividualBlock = ParentBlockInformation[block];
    compressor.TwoDCompression(parser, IndividualBlock, block);

    // std::cout << "\n";
  }


}