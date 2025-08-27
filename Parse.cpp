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
      NumXBlocks = Xcount / ParentX;
      NumYBlocks = Ycount / ParentY;
      NumZBlocks = Zcount / ParentZ;
    } else if (!map) {
      std::string location;
      char symbol;
      ss >> symbol >> delimeter >> location;
      TagTable[symbol] = location;

    } else if (map) {
      std::cout << Lines[i] << std::endl;
      std::cout << RLERow(Lines[i]) << std::endl;
      std::string *blocks = RLERowParent(Lines[i], ParentX, NumXBlocks);

      std::vector<std::string> Block2D;
      for (int blockNumT = 0; blockNumT < NumXBlocks; blockNumT++) {
        std::cout << " " << blocks[blockNumT] << "    ";
        Block2D.push_back(blocks[blockNumT]);
      }
      std::cout << "\n\n";
      layer.push_back(RLERow(Lines[i]));
      Blocks.push_back(Block2D);
    }
  }
  if (!layer.empty()) {
    MapInformation.push_back(layer);
  }
  std::vector<std::vector<std::vector<std::string>>> BlocksByLayer;
  int RowsPerLayer = Ycount;
  int CurrentLayer = 0;
  int RowsInCurrentLayer = 0;

  std::vector<std::vector<std::string>> CurrentLayerBlocks;
  for (size_t row = 0; row < Blocks.size(); row++) {
    CurrentLayerBlocks.push_back(Blocks[row]);
    RowsInCurrentLayer++;

    if (RowsInCurrentLayer == RowsPerLayer) {
      BlocksByLayer.push_back(CurrentLayerBlocks);
      CurrentLayerBlocks.clear();
      RowsInCurrentLayer = 0;
      CurrentLayer++;
    }
  }

  if (!CurrentLayerBlocks.empty()) {
    BlocksByLayer.push_back(CurrentLayerBlocks);
  }

  int TotalParentBlocks = (NumXBlocks) * (NumYBlocks) * (NumZBlocks);
  for (int ParentBlockIndex = 0; ParentBlockIndex < TotalParentBlocks;
       ParentBlockIndex++) {
    int BlockZ = ParentBlockIndex / (NumYBlocks * NumXBlocks);
    int BlockY = (ParentBlockIndex % (NumXBlocks * NumYBlocks)) / NumXBlocks;
    int BlockX = ParentBlockIndex % NumXBlocks;

    std::vector<std::vector<std::string>> CompleteBlock;
    for (int z = 0; z < ParentZ && (BlockZ * ParentZ + z) < static_cast<int>(BlocksByLayer.size()); z++) {
      int ActualZ = BlockZ * ParentZ + z;

      for (int y = 0; y < ParentY && (BlockY * ParentY + y) < static_cast<int>(BlocksByLayer[ActualZ].size()); y++) {
        int ActualY = BlockY * ParentY + y;

        if (BlockX < static_cast<int>(BlocksByLayer[ActualZ][ActualY].size())) {
          CompleteBlock.push_back({BlocksByLayer[ActualZ][ActualY][BlockX]});
        }
      }
    }
    if (!CompleteBlock.empty()) {
      ParentBlockInformation.push_back(CompleteBlock);
    }
  }
}


std::unordered_map<char, std::string> Parse::getTagTable() { return TagTable; }

std::string *Parse::RLERowParent(std::string Row, int ParentX, int NumXBlocks) {
  std::string *blocks = new std::string[NumXBlocks];
  int Counter = 0;
  int blockCounter = 0;
  int blockNum = 0;
  std::string TempString;
  char PrevChar;
  for (size_t i = 0; i < Row.length(); i++) {
    if (blockCounter == 0) {
      PrevChar = Row[i];
    }
    char CurrChar = Row[i];
    if (CurrChar == PrevChar) {
      Counter++;
      blockCounter++;
    } else {
      TempString += std::to_string(Counter) + PrevChar;
      PrevChar = CurrChar;
      Counter = 1;
      blockCounter++;
    }
    if (blockCounter == ParentX) {
      TempString += std::to_string(Counter) + PrevChar;
      blocks[blockNum] = TempString;
      TempString.clear();
      Counter = 0;
      blockCounter = 0;
      blockNum++;
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