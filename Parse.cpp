#include "Parse.h"

Parse::Parse() { Xcount = Ycount = Zcount = ParentX = ParentY = ParentZ = 0; }

Parse::Parse(std::vector<std::string> Lines) {
  char delimeter;
  std::string token;
  bool map = false;
  std::vector<std::string> layer;

  for (size_t i = 0; i < Lines.size(); i++) {
    std::istringstream ssCheck(Lines[i]);

    if (Lines[i].empty()) {
      if (!layer.empty()) {
        MapInformation.push_back(layer);
        layer.clear();
      }
    }

    if (!(ssCheck >> token)) {
      map = true;
      continue;
    }

    std::istringstream ss(Lines[i]);

    if (i == 0) {
      ss >> Xcount >> delimeter >> Ycount >> delimeter >> Zcount >> delimeter >>
          ParentX >> delimeter >> ParentY >> delimeter >> ParentZ;

    } else if (!map) {
      std::string location;
      char symbol;
      ss >> symbol >> delimeter >> location;
      TagTable[symbol] = location;

    } else if (map) {
      std::cout << Lines[i] << std::endl;
      std::cout << RLERow(Lines[i]) << std::endl;
      layer.push_back(RLERow(Lines[i]));

    }
  }
  if (!layer.empty()) {
    MapInformation.push_back(layer);
  }
}

std::unordered_map<char, std::string> Parse::getTagTable() { return TagTable; }

std::string Parse::RLERow(std::string Row) {
  std::string RLEString;
  int Counter = 0;
  char PrevChar = Row[0];

  for (size_t i = 0; i < Row.length(); i++) {
    char CurrChar = Row[i];
    if (CurrChar == PrevChar) {
      Counter++;
    } else {
      RLEString += std::to_string(Counter) + PrevChar;
      PrevChar = CurrChar;
      Counter = 1;
    }
  }

  RLEString += std::to_string(Counter) + PrevChar;
  return RLEString;
}

std::vector<std::vector<std::string>> Parse::GetMap() { return MapInformation; }