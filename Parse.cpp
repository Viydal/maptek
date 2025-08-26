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

char Parse::GetLetter(std::string encoded, int col) {
    bool notFound = true;
    char letter = ' ';
    int count = 0;
    int pos = 0;  // running position in expanded string

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
