#include "Parse.h"

Parse::Parse() { XCount = YCount = ZCount = ParentX = ParentY = ParentZ = 0; }

Parse::Parse(std::vector<std::string> Lines) {
    char Delimeter;
    std::string Token;
    bool Map = false;
    std::vector<std::string> Layer;

    for (size_t i = 0; i < Lines.size(); i++) {
        std::istringstream SsCheck(Lines[i]);

        if (Lines[i].empty()) {
            if (!Layer.empty()) {
                MapInformation.push_back(Layer);
                Layer.clear();
            }
        }

        if (!(SsCheck >> Token)) {
            Map = true;
            continue;
        }

        std::istringstream Ss(Lines[i]);

        if (i == 0) {
            Ss >> XCount >> Delimeter >> YCount >> Delimeter >> ZCount >> Delimeter
               >> ParentX >> Delimeter >> ParentY >> Delimeter >> ParentZ;
            NumXBlocks = XCount / ParentX;
            NumYBlocks = YCount / ParentY;
            NumZBlocks = ZCount / ParentZ;
        } else if (!Map) {
            std::string Location;
            char Symbol;
            Ss >> Symbol >> Delimeter >> Location;

            TagTable[static_cast<int>(Symbol)] = Location;
        } else if (Map) {
            Layer.push_back(RLERow(Lines[i]));
        }
    }
    if (!Layer.empty()) {
        MapInformation.push_back(Layer);
    }
}

std::string* Parse::GetTagTable() { return TagTable; }

std::string* Parse::RLERowParent(std::string Row, int ParentX, int NumXBlocks) {
    std::string* Blocks = new std::string[NumXBlocks];
    int Counter = 0;
    int BlockCounter = 0;
    int BlockNum = 0;
    char PrevChar = Row[0];
    std::string TempString;

    for (size_t i = 0; i < Row.length(); i++) {
        char CurrChar = Row[i];
        if (CurrChar == PrevChar) {
            Counter++;
            BlockCounter++;
        } else {
            TempString += std::to_string(Counter) + PrevChar;
            PrevChar = CurrChar;
            Counter = 1;
            BlockCounter++;
        }

        if (BlockCounter == ParentX) {
            TempString += std::to_string(Counter) + PrevChar;
            Blocks[BlockNum] = TempString;
            TempString.clear();
            PrevChar = CurrChar;
            Counter = 0;
            BlockCounter = 0;
            BlockNum++;
        }
    }
    return Blocks;
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

char Parse::GetLetter(std::string Encoded, int Col) {
    bool NotFound = true;
    char Letter = ' ';
    size_t Count = 0;
    int Pos = 0; // running position in expanded string

    // parse through the string
    while (NotFound && Count < Encoded.size()) {
        // read number
        int Num = 0;
        while (Count < Encoded.size() && isdigit(Encoded[Count])) {
            Num = Num * 10 + (Encoded[Count] - '0');
            Count++;
        }

        // read character
        char Ch = Encoded[Count];
        Count++;

        // check if Col falls inside this run
        if (Col < Pos + Num) {
            Letter = Ch;
            NotFound = false;
        }

        Pos += Num;
    }

    return Letter;
}

std::vector<std::vector<std::string>> Parse::GetMap() { return MapInformation; }
