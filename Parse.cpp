#include "Parse.h"

Parse::Parse() { XCount = YCount = ZCount = ParentX = ParentY = ParentZ = 0; }




Parse::Parse(std::vector<std::string> Lines) {
    // Used for cahing RLE results
    std::unordered_map<std::string, std::vector<std::pair<int,char>>> DP;

    //used for splitting input using string stream 
    char Delimeter;
    std::string Token;
    int Iterator = 0;

    // Read dimensions and parent block sizes from the first line
    std::istringstream SsCheck(Lines[Iterator]);
    std::istringstream Ss(Lines[Iterator]);
    Ss >> XCount >> Delimeter >> YCount >> Delimeter >> ZCount >> Delimeter
        >> ParentX >> Delimeter >> ParentY >> Delimeter >> ParentZ;
        NumXBlocks = XCount / ParentX;
        NumYBlocks = YCount / ParentY;
        NumZBlocks = ZCount / ParentZ;
    Iterator++;

    // Reads tag table, stops when it reaches a blank which indicates the start of the map
    std::string Location;
    char Symbol;
    while(Iterator < Lines.size()){
        std::istringstream SsCheck(Lines[Iterator]);
        std::istringstream Ss(Lines[Iterator]);
        if (!(SsCheck >> Token)){
            Iterator++;
            break;
        }
        Ss >> Symbol >> Delimeter >> Location;
        TagTable[static_cast<int>(Symbol)] = Location;
        Iterator++;
    }
        
    // Read the map information, separating layers by blank lines
    std::vector<Block> RowBlocks;
    std::vector<std::vector<Block>> LayerBlocks;
    int LayerNum = 0, YInLayer = 0;
    std::string Line;
    for (size_t i = Iterator; i < Lines.size(); i++) {
        Line = Lines[i];
        //if the line is blank, it indicates the end of a layer
        if (Line.empty()) {
            if (!LayerBlocks.empty()) {
                XBlocks.push_back(LayerBlocks);
                LayerBlocks.clear();
            }
            YInLayer = 0;
            LayerNum++;
            continue;
        
        }
        //want to convert each line to a block as we read it;  
        RowBlocks.clear();

        //check if the entire line is uniform
        bool uniform = true;
        char first = Line[0];
        for (size_t i = 1; i < Line.size(); i++) {
            if (Line[i] != first) uniform = false;
        }
        if (uniform){
            for (int startX = 0; startX < XCount; startX += ParentX) {
                int len = std::min(ParentX, XCount - startX);
                RowBlocks.push_back({ startX, YInLayer, LayerNum, len, 1, 1, first });
            }
            LayerBlocks.push_back(RowBlocks);
            ++YInLayer;
            continue;
        }
        
        //split the line into XBlocks of size ParentX
        for (int startX = 0; startX < XCount; startX += ParentX) {
            int len = std::min(ParentX, XCount - startX);
            std::string XBlockString = Line.substr(startX, len);

            //check if the entire block is uniform
            uniform = true;
            first = XBlockString[0];
            for (size_t i = 1; i < XBlockString.size(); i++) {
                if (XBlockString[i] != first) uniform = false;
            }
            //if (uniform){
            //    RowBlocks.push_back({ startX, YInLayer, LayerNum, len, 1, 1, first });
            //    continue;
            //}   
            //else, run RLE on the block
            RLERow(XBlockString, &RowBlocks, &DP,startX, YInLayer, LayerNum);
        }
        LayerBlocks.push_back(RowBlocks);
        YInLayer++;
        
    }
    if (!LayerBlocks.empty()) {
        XBlocks.push_back(LayerBlocks);
    }

}

std::string Parse::TestRLERow(std::string Row) {
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

void Parse::RLERow(std::string XBlockString, std::vector<Block> *RowBlocks, std::unordered_map<std::string, std::vector<std::pair<int,char>>> *DP,int StartX, int RowNum, int LayerNum) {
    //dynamic programming / caching approach to caching previously computed RLE results
    //commented out for now as it doesn't seem to improve performance
    /*
    if (DP->count(XBlockString)) {
        auto& Runs = DP->at(XBlockString);
        for (int i = 0; i < static_cast<int>(Runs.size()); i++) {
            int Length  = Runs[i].first;
            char Character = Runs[i].second;
            RowBlocks->push_back({ StartX, RowNum, LayerNum, Length, 1, 1, Character });
            StartX += Length;
        }
        return;
    }
    std::vector<std::pair<int,char>> Runs;
    */

    int Counter = 1;
    char CurrChar;
    char PrevChar = XBlockString[0];

    for (size_t i = 1; i < XBlockString.length(); i++) {
        CurrChar = XBlockString[i];
        if (CurrChar == PrevChar) {
            Counter++;
        } else {
            RowBlocks->push_back({StartX, RowNum, LayerNum, Counter, 1, 1, PrevChar});
            //Runs.push_back({Counter, PrevChar});
            PrevChar = CurrChar;
            StartX += Counter;
            Counter = 1;
        }
    }
    RowBlocks->push_back({StartX, RowNum, LayerNum, Counter, 1, 1, PrevChar});
    //Runs.push_back({Counter, PrevChar});
    
    //DP->insert({XBlockString, Runs});
}