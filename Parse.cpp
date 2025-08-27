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
      NumXBlocks = Xcount/ParentX;
      NumYBlocks = Ycount/ParentY;
      NumZBlocks = Ycount/ParentZ;
    } else if (!map) {
      std::string location;
      char symbol;
      ss >> symbol >> delimeter >> location;
      TagTable[symbol] = location;

    } else if (map) {
      // std::cout << Lines[i] << std::endl;
      // std::cout << RLERow(Lines[i]) << std::endl;
      std::string *blocks = RLERowParent(Lines[i], ParentX, NumXBlocks);
      // for (int blockNumT = 0; blockNumT < NumXBlocks; blockNumT++) {
      //   std::cout << " " << blocks[blockNumT] << "    ";
      // }
      // std::cout << "\n\n";
      layer.push_back(RLERow(Lines[i]));
    }
  }
  if (!layer.empty()) {
    MapInformation.push_back(layer);
  }
}

std::unordered_map<char, std::string> Parse::getTagTable() { return TagTable; }


std::string* Parse::RLERowParent(std::string Row, int ParentX, int NumXBlocks){
  std::string *blocks = new std::string[NumXBlocks];
  int Counter = 0;
  int blockCounter = 0;
  int blockNum = 0;
  char PrevChar = Row[0];
  std::string TempString;
  for (size_t i = 0; i < Row.length(); i++) {
    char CurrChar = Row[i];
    if (CurrChar == PrevChar) {
      Counter++;
      blockCounter ++;
    } else {
      TempString += std::to_string(Counter) + PrevChar;
      PrevChar = CurrChar;
      Counter = 1;
      blockCounter ++;
    }
    if (blockCounter == ParentX){
      TempString += std::to_string(Counter) + PrevChar;
      blocks[blockNum] = TempString;
      TempString.clear();
      PrevChar = CurrChar;
      Counter = 0;
      blockCounter = 0;
      blockNum ++;
    }
  }
  return blocks;
}


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

char Parse::GetLetter(std::string encoded, int col) {
  bool notFound = true;
  char letter = ' ';
  size_t count = 0;
  int pos = 0; // running position in expanded string

  // parse through the string
  while (notFound && count < encoded.size()) {
    // read number
    int num = 0;
    while (count < encoded.size() && isdigit(encoded[count])) {
      num = num * 10 + (encoded[count] - '0');
      count++;
    }

    // read character
    char ch = encoded[count];
    count++;

    // check if col falls inside this run
    if (col < pos + num) {
      letter = ch;
      // found the letter
      notFound = false;
    }

    pos += num;
  }

  return letter;
}

std::vector<std::vector<std::string>> Parse::GetMap() { return MapInformation; }