#include "Parse.h"
 
Parse::Parse(const std::vector<std::string>& Lines) {
    this->Lines = Lines;
    ParseHeader();
    ParseMap();

}


int Parse::ParseHeader(){
    //used for splitting input using string stream 
    char Delimeter;
    std::string Token;
    Iterator = 0;

    //Read dimensions and parent block sizes from the first line
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
    return Iterator;
}


void Parse::ParseMap(){
    //caches
    std::unordered_map<std::string, std::vector<Block>> CompressionCache2d;
    std::unordered_map<std::string, std::vector<Block>> CompressionCache3d;
    std::unordered_map<std::string, std::vector<std::pair<int,char>>> RleCache;
    CompressionCache2d.reserve(4096);
    CompressionCache3d.reserve(4096);
    RleCache.reserve(4096);

    
    Compression Compressor = Compression();
    std::ostringstream Output;
    OutputBlocks.reserve((XCount * YCount * ZCount) * 0.8);
    startX = 0, startY = 0, startZ = 0;
    
    for (int z = 0; z < NumZBlocks; z++) {
        for (int y = 0; y < NumYBlocks; y++) {
            for (int x = 0; x < NumXBlocks; x++) {
            startX = x * ParentX;
            startY = y * ParentY;
            startZ = z * ParentZ;

        
        // Create a key for caching a parent block by concatenating its input lines into a large string
        char MapKey3d[ParentX * ParentY * ParentZ];
        Create3dKey(MapKey3d);

        // check if the entire string is uniform and skip compressing if so
        if (UniformCheck(MapKey3d)){
            OutputBlocks.push_back({startX, startY, startZ, ParentX, ParentY, ParentZ, MapKey3d[0]});
            continue; // move onto next parent block
        }

        // check if the parent block is in the cache and skip compressing if so
        auto it = CompressionCache3d.find(MapKey3d);
        if (it != CompressionCache3d.end()) {
            OutputBlocks.push_back(it->second);
            OutputBlocks.back().StartX = startX;
            OutputBlocks.back().StartY = startY;
            OutputBlocks.back().StartZ = startZ;
            continue; //move onto next parent block
        }

        ParentBlock IndividualParentBlock(startX, startY, startZ, ParentX, ParentY, ParentZ);


        // Try to compress the Parent X by Parent Y by 1 slices
        // There are ParentZ no. of slices in each parent block
        for (int localZ = 0; localZ < ParentZ; localZ++) {
            // Create a key for caching the slice by concatenating its input lines into a large string
            char MapKey2d[ParentX * ParentY];
            Create2dKey(MapKey2d, localZ); 

            // check if the entire string is uniform and skip compressing if so
            if (UniformCheck(MapKey2d)){
                IndividualParentBlock.Blocks.push_back({0, 0, localZ, ParentX, ParentY, 1, MapKey2d[0]});
                continue;// move onto next slice
            }



            // check if the slice is in the cache and 2d skip compressing if so
            auto it = CompressionCache2d.find(MapKey2d);
            if (it != CompressionCache2d.end()) {
                auto Blocks = it->second;
                for (auto& Block : Blocks) {
                    Block.ZPos = localZ;
                    IndividualParentBlock.Blocks.push_back(Block); // slice is added to list of blocks to be 3d compressed
                }
                continue;// move onto next slice
            }

            //None of the shortcuts have worked, X then XY then XYZ compression will take place
            //Perfrom X compression on the slice using rle
            std::vector<Block> ParentSlice;
            ParentSlice.reserve(ParentX * ParentY);
            for (int Y = 0; Y < ParentY; Y++) {
                int XBlockStart = Y * ParentX;

                char Row[ParentX];
                for (int j = 0; j < ParentX; j++){
                    Row[j] = MapKey2d[XBlockStart + j];
                }
                if (UniformCheck(Row)){
                    ParentSlice.push_back({0, Y, localZ, ParentX, 1, 1, Row[0]});
                    continue;
                }
                RLERow(&Row[0],&ParentSlice, &RleCache,0, Y, localZ);
            }
            IndividualParentBlock.Blocks.insert(IndividualParentBlock.Blocks.end(), ParentSlice.begin(), ParentSlice.end());

            // Add newly computed Parent Slice compression to the cache
            CompressionCache2d.insert({MapKey2d, ParentSlice});

            }
            // the saved answer is stored in the cache
            CompressionCache3d.insert({MapKey3d, IndividualParentBlock.Blocks});

            OutputBlocks.push_back(IndividualParentBlock);
            // The process will now repeat
            }
        }
}
    
    return;
}


std::string Parse::CollectOutput(std::vector<ParentBlock>& ParentBlocks) {
    std::string Output;
    for (ParentBlock &PB : ParentBlocks){
        Output += PB.WriteBlock(TagTable);
    }
    return Output;
}


void Parse::Create3dKey(char* key3d) {
    size_t writePos = 0;
    const size_t stridePerLayer = YCount + 1; // if you truly have a blank line between layers
    const size_t total = static_cast<size_t>(ParentX) * ParentY * ParentZ;

    // (Optional) sanity checks
    // assert(key3d != nullptr);
    // assert(startX >= 0 && startY >= 0 && startZ >= 0);
    // assert(startX + ParentX <= XCount);
    // assert(startY + ParentY <= YCount);
    // assert(startZ + ParentZ <= NumZBlocks /* or ZCount, as appropriate */);

    for (int z = 0; z < ParentZ; ++z) {
        const int base = Iterator + (startZ + z) * static_cast<int>(stridePerLayer);
        for (int y = 0; y < ParentY; ++y) {
            const int lineIdx = base + (startY + y);
            const std::string& line = Lines[lineIdx];

            std::memcpy(key3d + writePos, line.data() + startX, ParentX);
            writePos += ParentX;
        }
    }
}

void Parse::Create2dKey(char * Key2d, int localZ){
    size_t writePos = 0;
    const size_t base = Iterator + (startZ + localZ) * (YCount + 1);
        for (int y = 0; y < ParentY; ++y) {
            const int lineIdx = base + (startY + y);
            const std::string& line = Lines[lineIdx];

            std::memcpy(Key2d + writePos, line.data() + startX, ParentX);
            writePos += ParentX;
        }
}

bool Parse::UniformCheck(char * Key){
    char first = Key[0];
    for (int i = 1; i < ParentX * ParentY * ParentZ; i++){
        if (Key[i] != first)
        return false;
    }
    return true;
}

void Parse::Create2dKey(std::string& Key2d, int localZ){
    Key2d.reserve(ParentX * ParentY);
    for (int y = 0; y < ParentY; y++) {
        int lineIdx =  Iterator + ((startZ + localZ) * (YCount + 1)) + startY + y;
        Key2d.append(Lines[lineIdx], startX, ParentX);
    }
}
bool Parse::UniformCheck(std::string& Key){
    char first = Key[0];
    for (int i = 1; i < Key.size(); i++){
        if (Key[i] != first)
        return false;
    }
    return true;
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

void Parse::RLERow(char* BlockString, std::vector<Block> *RowBlocks, std::unordered_map<std::string, std::vector<std::pair<int,char>>> *RleCache,int StartX, int RowNum, int LayerNum) {
    //dynamic programming / caching approach to caching previously computed RLE results
    //commented out for now as it doesn't seem to improve performance
    
    if (RleCache->count(BlockString)) {
        auto& Runs = RleCache->at(BlockString);
        for (int i = 0; i < Runs.size(); i++) {
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
    char PrevChar = BlockString[0];

    for (size_t i = 1; i < ParentX; i++) {
        CurrChar = BlockString[i];
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
    
    // if (RleCache->size() > 4096){
    //     RleCache->clear();
    //     RleCache->reserve(4096);
    //     RleCache->rehash(4096);
    // }
    RleCache->insert({BlockString, Runs});
    
}