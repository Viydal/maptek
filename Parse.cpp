#include "Parse.h"

Parse::Parse() { Xcount = Ycount = Zcount = ParentX = ParentY = ParentZ = 0; }

Parse::Parse(std::vector<std::string> lines) {
  char delimeter;
  std::string token;

  for (size_t i = 0; i < lines.size(); i++) {
    std::istringstream ssCheck(lines[i]);

    if (!(ssCheck >> token)) {
      break;
    }

    std::istringstream ss(lines[i]);

    if (i == 0) {
      ss >> Xcount >> delimeter >> Ycount >> delimeter >> Zcount >> delimeter >>
          ParentX >> delimeter >> ParentY >> delimeter >> ParentZ;
    } else {
      std::string location;
      char symbol;
      ss >> symbol >> delimeter >> location;
      TagTable[symbol] = location;
    }
  }
}

std::unordered_map<char, std::string> Parse::getTagTable() { return TagTable; }