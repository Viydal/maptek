#include "Parse.h"

Parse::Parse() { XCount = YCount = ZCount = ParentX = ParentY = ParentZ = 0; }

Parse::Parse(std::vector<std::string> Lines) {
    // Used for cahing RLE results
    std::unordered_map<std::string, std::vector<std::pair<int,char>>> RleCache;
    RleCache.reserve(4096);
    //robin_hood::unordered_flat_map<std::string, std::vector<std::pair<int,char>>> RleCache;
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
   
    std::unordered_map<std::string, std::vector<std::vector<Block>>> CompressionCache;
    CompressionCache.reserve(4096);
    

    // Read the map information, separating layers by blank lines
    
    std::vector<Block> OutputBlocks;

    int startX = 0;
    int startY = 0;
    int NumXYParentBlocks = (NumXBlocks) * (NumYBlocks);

    Compression Compressor = Compression();
    std::ostringstream Output;

    Cache2dHit = 0, Cache2dMiss = 0;

    for (int i = 0; i < ZCount; i++){
        for (int j = 0; j < NumXYParentBlocks; j++){
            startX = (j % NumXBlocks) * ParentX;
            startY = (j / NumXBlocks) * ParentY;

            std::string MapKey;
            for (int y = Iterator + startY; y < Iterator + startY + ParentY; y++){
                MapKey += Lines[y].substr(startX, ParentX);
            }
            //MapKey = TestRLERow(MapKey);


            if (CompressionCache.count(MapKey)) {
                Cache2dHit ++;
                auto& Blocks = CompressionCache.at(MapKey);
                for (int k = 0; k < Blocks.size(); k++){
                    for (int l = 0; l < Blocks[k].size(); l++) {
                        Block NewBlock = Blocks[k][l];
                        NewBlock.XPos += startX;
                        NewBlock.YPos += startY;
                        NewBlock.ZPos = i;
                        OutputBlocks.push_back(NewBlock);
                    }
                }
            }else{
                Cache2dMiss++;
                std::vector<std::vector<Block>> ParentBlock(ParentY);
                for (int y = Iterator + startY; y < Iterator + startY + ParentY; y++){
                    int LocalY = y - Iterator - startY;
                    std::string StringRow = Lines[y].substr(startX, ParentX);
                    RLERow(&StringRow[0], &ParentBlock[LocalY], &RleCache, 0, LocalY, i);

                }

                // this method of getting the ProcessLayer results is shameleslly copied from chatgpt
                size_t prevSize = Compressor.GetBlocksSize();
                Compressor.ProcessLayer(ParentBlock, ParentX, ParentY, ParentZ, i, Output, TagTable);
                const auto& all = Compressor.GetBlocks();
                std::vector<Block> merged(all.begin() + prevSize, all.end()); // merged, RELATIVE blocks
                
                if (CompressionCache.size() > 4096){
                    CompressionCache.clear();
                    CompressionCache.reserve(4096);
                    CompressionCache.rehash(4096);
                }
                std::vector<std::vector<Block>> mergedRows;
                mergedRows.push_back(merged);
                CompressionCache.insert({MapKey, mergedRows});
                
                for (int k = 0; k < merged.size(); k++){
                    Block NewBlock = merged[k];
                    NewBlock.XPos += startX;
                    NewBlock.YPos += startY;
                    NewBlock.ZPos = i;

                    OutputBlocks.push_back(NewBlock);
                }
                
            }
    }

        Iterator += YCount + 1;
    }
    std::vector<Block> OutputZMerge;

    //std::cout<<"\n \n Ouput: \n";
    Compressor.MergeLayers(OutputBlocks, ParentZ);
    std::cout << "Z-merge: in=" << OutputBlocks.size()
          << " out=" << Compressor.GetFinalBlocks().size() << "\n";
    //Compressor.WriteBlocks(Compressor.GetFinalBlocks(), Output, TagTable);
    std::cout << Output.str();
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



void Parse::RLERow(char* XBlockString, std::vector<Block> *RowBlocks, std::unordered_map<std::string, std::vector<std::pair<int,char>>> *RleCache,int StartX, int RowNum, int LayerNum) {
    //dynamic programming / caching approach to caching previously computed RLE results
    //commented out for now as it doesn't seem to improve performance
    
    if (RleCache->count(XBlockString)) {
        auto& Runs = RleCache->at(XBlockString);
        for (int i = 0; i < static_cast<int>(Runs.size()); i++) {
            int Length  = Runs[i].first;
            char Character = Runs[i].second;
            RowBlocks->push_back({ StartX, RowNum, LayerNum, Length, 1, 1, Character });
            StartX += Length;
        }
        return;
    }
    
    /*
    std::string key(XBlockString, ParentX);
    std::unordered_map<std::string, std::vector<std::pair<int,char>>>::iterator it = RleCache->find(key);
    if (it != RleCache->end()) {
        std::vector<std::pair<int,char>>& Runs = it->second;
        for (int i = 0; i < Runs.size(); i++){
            int Length = Runs[i].first;
            char Character = Runs[i].second;
            RowBlocks->push_back({ StartX, RowNum, LayerNum, Length, 1, 1, Character});
            StartX += Length;
        }
        return;
    }
    */

    std::vector<std::pair<int,char>> Runs;

    int Counter = 1;
    char CurrChar;
    char PrevChar = XBlockString[0];
    int len = std::min(ParentX, XCount - StartX);

    for (size_t i = 1; i < len; i++) {
        CurrChar = XBlockString[i];
        if (CurrChar == PrevChar) {
            Counter++;
        } else {
            RowBlocks->push_back({StartX, RowNum, LayerNum, Counter, 1, 1, PrevChar});
            Runs.push_back({Counter, PrevChar});
            PrevChar = CurrChar;
            StartX += Counter;
            Counter = 1;
        }
    }
    RowBlocks->push_back({StartX, RowNum, LayerNum, Counter, 1, 1, PrevChar});
    Runs.push_back({Counter, PrevChar});
    
    if (RleCache->size() > 4096){
        RleCache->clear();
        RleCache->reserve(4096);
        RleCache->rehash(4096);
    }
    RleCache->insert({XBlockString, Runs});
    
}

