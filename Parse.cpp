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
    int saveIterator = Iterator;


     /*
     std::unordered_map<std::string, std::vector<Block>> CompressionCache;
    CompressionCache.reserve(4096 * 2);
    std::string MapKey;

            if (CompressionCache.size() > 4096){
                CompressionCache.clear();
                CompressionCache.reserve(4096);
                CompressionCache.rehash(4096);
            }
            CompressionCache.insert({MapKey, ParentBlock});
            */
    /*
            if (CompressionCache.count(MapKey)) {
                auto& Blocks = CompressionCache.at(MapKey);
                for (int k = 0; k < Blocks.size(); k++) {
                    Blocks[i].XPos = j % NumXBlocks * ParentY + Blocks[i].YPos;
                    Blocks[i].YPos = j / NumXBlocks * ParentY + Blocks[i].YPos;
                    Blocks[i].ZPos = k;

                    ParentBlock.push_back(Blocks[i]);
                }
                std::cout<<"Cache hit"<<std::endl;
                LayerParentBlocks.push_back(ParentBlock);
                std::cout<<LayerParentBlocks.size()<<std::endl;
                for (int f = 0;f < LayerParentBlocks.size(); f++){
                    for (int g = 0;g< LayerParentBlocks[f].size(); g++){
                        std::cout<<LayerParentBlocks[f][g].Ch<<"\n";
                    }
                }
                if (startX + 2 * ParentX <= XCount){
                    startX += ParentX;
                }else{
                    startX = 0;
                    startY += ParentY;
                }
                ParentBlock.clear();
                continue;
            }
            */

    // Read the map information, separating layers by blank lines
    std::vector<std::vector<std::vector<Block>>> AllParentBlocks;
    std::vector<std::vector<Block>> LayerParentBlocks;
    std::vector<Block> ParentBlock;
    std::vector<std::vector<std::string>> LayerStringParentBlocks;
    std::vector<std::string> StringParentBlock;
    
    //trick process layer
    std::vector<std::vector<Block>> TrickProcessLayer;

    int startX = 0;
    int startY = 0;
    std::string StringBlock;
    int NumXYParentBlocks = (NumXBlocks) * (NumYBlocks);

    Compression Compressor = Compression();
    std::ostringstream Output;

    for (int i = 0; i < ZCount; i++){
        for (int j = 0; j < NumXYParentBlocks; j++){
            //std::cout<<"j = "<< j <<"Should be less than "<<NumXYParentBlocks<<std::endl;

            //MapKey = "";
            //create stringparent block
            
            for (int y = Iterator + startY; y < Iterator + startY + ParentY; y++){
                //std::cout<<"Line number:"<<y<<"\n";
                //std::cout<<Lines[y]<<"\n";
                StringBlock = Lines[y].substr(startX, ParentX);
                StringParentBlock.push_back(StringBlock);
            }
            

            //for (int k = 0; k < StringParentBlock.size(); k++) {
            //    MapKey += StringParentBlock[k].first;
            //}
            //std::cout<<"Layer: "<<i<<" Block: "<< j <<"\n"<< MapKey <<std::endl;
            
            //create parentblock
            std::cout<<"Parent Block no Merge \n";
            for (size_t xx = 0; xx < StringParentBlock.size(); xx++){
                std::cout<<StringParentBlock[xx]<<"\n";
            }

            for (int k = 0; k < StringParentBlock.size(); k++) {
                RLERow(&StringParentBlock[k][0], &ParentBlock, &RleCache, 0, k, i);
            }

            std::cout<<"\nParent Block X Merge \n";
            for (size_t xx = 0; xx < ParentBlock.size(); xx++){
                
                std::cout<<ParentBlock[xx].XPos<<",";
                std::cout<<ParentBlock[xx].YPos<<",";
                std::cout<<ParentBlock[xx].ZPos<<",";
                std::cout<<ParentBlock[xx].XSize<<",";
                std::cout<<ParentBlock[xx].YSize<<",";
                std::cout<<ParentBlock[xx].ZSize<<",";
                std::cout<<ParentBlock[xx].Ch<<"\n";
            }
            TrickProcessLayer.push_back(ParentBlock);
            Compressor.ProcessLayer(TrickProcessLayer, ParentX, ParentY, ParentZ, i, Output, TagTable);
            TrickProcessLayer.clear();
            std::cout<<"\n""Parent Block XY Merge "<<"\n";
            for (size_t xx = 0; xx < ParentBlock.size(); xx++){
                std::cout<<ParentBlock[xx].XPos<<",";
                std::cout<<ParentBlock[xx].YPos<<",";
                std::cout<<ParentBlock[xx].ZPos<<",";
                std::cout<<ParentBlock[xx].XSize<<",";
                std::cout<<ParentBlock[xx].YSize<<",";
                std::cout<<ParentBlock[xx].ZSize<<",";
                std::cout<<ParentBlock[xx].Ch<<"\n";
            }


           

            LayerParentBlocks.push_back(ParentBlock);
            //LayerStringParentBlocks.push_back(StringParentBlock);
            ParentBlock.clear();
            StringParentBlock.clear();


            if (startX + 2 * ParentX <= XCount){
                startX += ParentX;
            }else{
                startX = 0;
                startY += ParentY;
            }
        }
        AllParentBlocks.push_back(LayerParentBlocks);
        LayerParentBlocks.clear();
        startY++;
    }
    
    std::cout << "WHAT IS GOING ON"<<std::endl;
    Compressor.WriteBlocks(Compressor.GetBlocks(), Output, TagTable);
    std::cout << Output.str();
    std::cout << "WHAT IS GOING ON"<<std::endl;
    
   
    
    std::cout<<std::endl;
    std::cout<<std::endl;
    std::cout<<std::endl;
    
   /*
    Iterator = saveIterator;
    
// Read the map information, separating layers by blank lines
    std::vector<Block> RowBlocks;
    RowBlocks.reserve(XCount);
    std::vector<std::vector<Block>> LayerBlocks;
    LayerBlocks.reserve(YCount);
    XBlocks.reserve(ZCount);

    int LayerNum = 0, YInLayer = 0;
    std::string Line;
    for (size_t i = Iterator; i < Lines.size(); i++) {
        Line = Lines[i];
        //if the line is blank, it indicates the end of a layer
        if (Line.empty()) {
            if (!LayerBlocks.empty()) {
                XBlocks.push_back(LayerBlocks);
                LayerBlocks.clear();
                RleCache.clear();
                RleCache.reserve(4096);
                RleCache.rehash(4096);
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
            if (Line[i] != first){
                uniform = false;
                break;
            } 
        }
        
        if (uniform){
            for (int startX = 0; startX < XCount; startX += ParentX) {
                int len = std::min(ParentX, XCount - startX);
                RowBlocks.emplace_back(startX, YInLayer, LayerNum, len, 1, 1, first);
            }
            LayerBlocks.push_back(RowBlocks);
            ++YInLayer;
            continue;
        }

        //split the line into XBlocks of size ParentX
        for (int startX = 0; startX < XCount; startX += ParentX) {
            int len = std::min(ParentX, XCount - startX);
            char XBlockString[len];
            for (int i = 0; i < len; i++) {
                XBlockString[i] = Line[startX + i];
            }

            //check if the entire block is uniform
            uniform = true;
            first = XBlockString[0];
            for (size_t i = 1; i < len; i++) {
                if (XBlockString[i] != first){
                    uniform = false;
                    break;
                } 
            }
            if (uniform){
                RowBlocks.push_back({ startX, YInLayer, LayerNum, len, 1, 1, first });
                continue;
            }   

            //else, run RLE on the block
            RLERow(&XBlockString[0], &RowBlocks, &RleCache,startX, YInLayer, LayerNum);
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
   /*
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
    */
}

