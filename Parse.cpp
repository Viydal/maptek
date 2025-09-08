#include "Parse.h"
#include "ParentBlock.h"
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
    int startZ = 0;
    int NumParentBlocks = NumXBlocks * NumYBlocks * NumZBlocks;
    int NumXYParentBlocks = NumXBlocks * NumYBlocks;

    Compression Compressor = Compression();
    std::ostringstream Output;


    
   
    std::vector<ParentBlock> ParentBlocks;
    ParentBlock IndividualParentBlock;
    
    for (int i = 0; i < NumZBlocks; i++) {
    for (int j = 0; j < NumXYParentBlocks; j++) {
        int startX = (j % NumXBlocks) * ParentX;
        int startY = (j / NumXBlocks) * ParentY;
        int startZ = i * ParentZ;

        for (int localZ = 0; localZ < ParentZ; localZ++) {

            std::string MapKey;
            MapKey.reserve(ParentX * ParentY);

            for (int y = 0; y < ParentY; ++y) {
                const int lineIdx =  Iterator + (startZ + localZ) * (YCount + 1) + startY + y;
                MapKey += Lines[lineIdx].substr(startX, ParentX);
            }

            const char first = MapKey[0];
            if (MapKey.find_first_not_of(first) == std::string::npos) {
                IndividualParentBlock.Blocks.push_back({ startX, startY, startZ + localZ, ParentX, ParentY, 1, first});
                continue;
            }

            auto it = CompressionCache.find(MapKey);
            if (it != CompressionCache.end()) {
                auto Rows = it->second;
                for (auto& Row : Rows) {
                    for (Block Block : Row) {
                        Block.XPos += startX;
                        Block.YPos += startY;
                        Block.ZPos = startZ + localZ;
                        IndividualParentBlock.Blocks.push_back(Block);
                    }
                }
                continue;
            }

            std::vector<std::vector<Block>> ParentSlice(ParentY);
            for (int y = 0; y < ParentY; ++y) {
                const int lineIndex = Iterator + (startZ + localZ) * (YCount + 1) + startY + y;
                std::string row = Lines[lineIndex].substr(startX, ParentX);
                RLERow(&row[0],&ParentSlice[y], &RleCache,0, y, localZ);
            }

            const size_t prev = Compressor.GetBlocksSize();
            Compressor.ProcessLayer(ParentSlice, ParentX, ParentY, ParentZ, i, Output, TagTable);
            auto& all = Compressor.GetBlocks();
            std::vector<Block> merged(all.begin() + prev, all.end());
            
            

            
            std::vector<std::vector<Block>> mergedRows;
            mergedRows.push_back(merged);

            CompressionCache.insert({MapKey, mergedRows});
            
            Block NewBlock;
            for (int k = 0; k < merged.size(); k++) {
                NewBlock = merged[k];
                NewBlock.XPos += startX;
                NewBlock.YPos += startY;
                NewBlock.ZPos += startZ;
                IndividualParentBlock.Blocks.push_back(NewBlock);
            }
            
        } 
        Compressor.MergeLayers(IndividualParentBlock.Blocks, ParentZ);
        OutputBlocks.insert(OutputBlocks.end(), Compressor.GetFinalBlocks().begin(), Compressor.GetFinalBlocks().end());
        
        IndividualParentBlock.Blocks.clear();
    }
    
}

    //std::cout<<"\n \n Ouput: \n";
    
     //std::cout << "Z-merge: in=" << OutputBlocks.size()
          // << " out=" << Compressor.GetFinalBlocks().size() << "\n";
    Compressor.WriteBlocksString(OutputBlocks, Output, TagTable);
    OutputBlocks.clear();
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

