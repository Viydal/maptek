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

void Parse::NoRLE(std::string_view BlockString, std::vector<Block>& RowBlocks, int StartX, int RowNum, int LayerNum){
    for (char C : BlockString){
        RowBlocks.push_back({ StartX, RowNum, LayerNum, 1, 1, 1, C });
        StartX ++;
    }
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
    OutputBlocks.reserve(NumXBlocks * NumYBlocks * NumZBlocks);
    startX = 0, startY = 0, startZ = 0;
    
    for (int i = 0; i < NumZBlocks; i++) {
        for (int j = 0; j < NumXBlocks * NumYBlocks; j++) {
            startX = (j % NumXBlocks) * ParentX;
            startY = (j / NumXBlocks) * ParentY;
            startZ = i * ParentZ;

        
        // Create a key for caching a parent block by concatenating its input lines into a large string
        std::string MapKey3d;
        Create3dKey(MapKey3d);

        // check if the entire string is uniform and skip compressing if so
        if (UniformCheck(MapKey3d)){
            OutputBlocks.push_back(ParentBlock(startX, startY, startZ, ParentX, ParentY, ParentZ, MapKey3d[0]));
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
            std::string MapKey2d;
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
            for (int Y = 0; Y < ParentY; Y++) {
                int XBlockStart = Y * ParentX;
                std::string Row = MapKey2d.substr(XBlockStart, ParentX);
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
    
    return;
}


std::string Parse::CollectOutput(std::vector<ParentBlock> ParentBlocks) {
    std::string Output;
    for (ParentBlock &PB : ParentBlocks){
        Output += PB.WriteBlock(TagTable);
    }
    return Output;
}


/*
// Perform 2d Compression on the newly X compressed Parent Slice
// Silly code with getblocks() is to retrieve the answer as
// process layer was not built to return the answer per block
const size_t Prev = Compressor.GetBlocksSize();
Compressor.ProcessLayer(ParentSlice, ParentX, ParentY, ParentZ, localZ, Output, TagTable);
std::vector<Block>& All = Compressor.GetBlocks();
std::vector<Block> Merged(All.begin() + Prev, All.end());



IndividualParentBlock.Blocks.reserve(IndividualParentBlock.Blocks.size() + Merged.size());
IndividualParentBlock.Blocks.insert(IndividualParentBlock.Blocks.end(), Merged.begin(), Merged.end());
}

// The merged slices are sent to 3d compression and there answer is saved
Compressor.MergeLayers(IndividualParentBlock.Blocks, ParentZ);
std::vector<Block> FinalLocal = Compressor.GetFinalBlocks();
IndividualParentBlock.Blocks.clear();
// the saved answer is stored in the cache
//CompressionCache3d.insert({MapKey3d, FinalLocal});

//Block coordinates are updated from relative to absolute
Block NewBlock;
for (int k = 0; k < FinalLocal.size(); k++) {
NewBlock = FinalLocal[k];
NewBlock.XPos += startX;
NewBlock.YPos += startY;
NewBlock.ZPos += startZ;
OutputBlocks.push_back(NewBlock); //blocks placed in output
*/


void Parse::Create3dKey(std::string& Key3d){
    Key3d.reserve(ParentX * ParentY * ParentZ);
    for (int z = 0; z < ParentZ; z++){
        for (int y = 0; y < ParentY; y++) {
            int lineIdx =  Iterator + ((startZ + z) * (YCount + 1)) + startY + y;
            Key3d.append(Lines[lineIdx], startX, ParentX);
        }
    }
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