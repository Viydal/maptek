#include "Tester.h"
#include "Compression.h"
#include "Parse.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <cstdio>


//chat gpt windows output speedup
#ifdef _WIN32
  #define NOMINMAX
  #include <io.h>    // _setmode, _fileno
  #include <fcntl.h> // _O_BINARY
#endif

inline int alloc_calls = 0;
inline int alloc_bytes = 0;

// void* operator new(std::size_t sz) {
//   ++alloc_calls; alloc_bytes += (int)sz;
//   return std::malloc(sz);
// }

// void* operator new[](std::size_t sz) {
//   ++alloc_calls; alloc_bytes += (long long)sz;
//   return std::malloc(sz);
// }

// void* operator new(std::size_t sz, std::align_val_t al) {
//   ++alloc_calls; alloc_bytes += (long long)sz;
// #ifdef _WIN32
//   void* p = _aligned_malloc(sz, (std::size_t)al);
// #else
//   void* p = nullptr;
//   if (posix_memalign(&p, (std::size_t)al, sz) != 0) p = nullptr;
// #endif
//   return p;
// }

int main(int argc, char* argv[]) {

    auto TotalProgramTime = std::chrono::high_resolution_clock::now();
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    //chat gpt windows output speedup
    #ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY); // no \n->\r\n translation
        _setmode(_fileno(stdin),  _O_BINARY);

        static char outbuf[1<<20];
        setvbuf(stdout, outbuf, _IOFBF, sizeof(outbuf));

        static char inbuf[1<<20];
        setvbuf(stdin,  inbuf,  _IOFBF, sizeof(inbuf));
    #endif

    Args Args;
    // --- Parse args ---
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f" && i + 1 < argc) {
            Args.readFile = true;
            Args.filePath = argv[++i];
        } else if (arg == "-t" || arg == "-ta") {
            Args.TestingMode = true;
            Args.TestAll = (arg == "-ta");
        } else if (arg == "-v"){
            if(i + 1 < argc && isdigit(argv[i + 1][0])) {
                Args.verboseLevel = std::stoi(argv[++i]);
                Args.verbose = true;
            } else {
                Args.verbose = true;
                Args.verboseLevel = 1;
            }
        } else {
            std::cerr << "Unknown or invalid argument: " << arg << "\n";
            return 1;
        }
    }
    if (Args.TestingMode) {
        cout << "Testing mode enabled." << endl;
        if (Args.TestAll) {
            std::cout << "Running all tests in TestCases/ directory.\n";
        }
    // Implement testing mode logic here
    }
    if (Args.readFile) {
        cout << "Reading file: " << Args.filePath << endl;
    // Implement file reading logic here
    }
    if (Args.verbose) {
        cout << "Verbose output enabled." << endl;
    // Implement verbose output logic here
    }
  
    // --- TESTING MODE ---
    if (Args.TestingMode) {

        if (Args.TestAll) {
            Tester::RunAllTests(Args);
        } else {

            if (!Args.readFile) {
                std::cerr << "Error: -t requires a file (-f <file>) or use -ta\n";
                return 1;
            }
            Tester::RunTest(Args);
        }
        return 0;
    }

    // --- NORMAL MODE ---

    std::unique_ptr<std::ifstream> file;
    std::istream* in = &std::cin; 

    if (Args.readFile) {
        file = std::make_unique<std::ifstream>("TestCases/" + Args.filePath);
        if (!*file) {
            std::cerr << "Error: could not open " << Args.filePath << "\n";
            return 1;
        }
        in = file.get();
    }
   
    auto startParseHeader = std::chrono::high_resolution_clock::now();
        
    Parse Parser;
    Parser.StreamParseHeader(*in);

    auto endParseHeader = std::chrono::high_resolution_clock::now();
    auto ParseHeaderDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endParseHeader - startParseHeader);

    auto startInitialiseParentBlocks = std::chrono::high_resolution_clock::now();
    std::vector<ParentBlock> ParentBlocks;
    ParentBlocks.reserve(Parser.NumXBlocks * Parser.NumYBlocks);

    for (int BlockRow = 0; BlockRow < Parser.NumYBlocks; BlockRow++) {
        for (int BlockCol = 0; BlockCol < Parser.NumXBlocks; BlockCol++) {
            ParentBlocks.emplace_back(BlockCol * Parser.ParentX, BlockRow * Parser.ParentY, 0, Parser.ParentX, Parser.ParentY, Parser.ParentZ);
        }
    }


    auto endInitialiseParentBlocks = std::chrono::high_resolution_clock::now();
    auto InitialiseParentBlocksDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endInitialiseParentBlocks - startInitialiseParentBlocks);

    Compression Compressor;

    std::unordered_map<std::string, std::vector<std::pair<int,char>>> RleCache;

    int ParseTime = 0, CompressTime = 0, WriteTime = 0;
    std::string Output;
    Output.reserve(ParentBlocks.size() * 80);

    for (int i = 0; i < Parser.NumZBlocks; i++){
        Output.clear();
        for (auto& pb : ParentBlocks) {
            pb.StartZ = i * Parser.ParentZ; 
            pb.Blocks.clear();
        }

        auto start = std::chrono::high_resolution_clock::now();
        Parser.StreamParseMapChunk(ParentBlocks, i, *in, RleCache);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        ParseTime += duration.count();

        start = std::chrono::high_resolution_clock::now();
        for (auto& PB : ParentBlocks) {
            Compressor.CompressParentBlock(PB);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        CompressTime += duration.count();

        start = std::chrono::high_resolution_clock::now();
        for (auto& PB : ParentBlocks) {
            PB.WriteBlock(Parser.TagTable, Output);
        }
        std::cout.write(Output.data(), Output.size());
        
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        WriteTime += duration.count();
    }
    auto TotalProgramEnd = std::chrono::high_resolution_clock::now();
    auto TotalProgramDuration = std::chrono::duration_cast<std::chrono::milliseconds>(TotalProgramEnd - TotalProgramTime);

    std::cout << "Total program done in " << TotalProgramDuration.count() << " ms.\n";
    std::cout << "Parse Header done in " << ParseHeaderDuration.count() << " ms.\n";
    std::cout << "Initialise Parent Blocks done in " << InitialiseParentBlocksDuration.count() << " ms.\n";
    std::cout << "Parsing done in " << ParseTime << " ms.\n";
    std::cout << "Compressing done in " << CompressTime << " ms.\n";
    std::cout << "Writing done in " << WriteTime << " ms.\n";
    std::cout << moveCounter << " ParentBlock or Block moves.\n";
    std::cout << copyCounter << " ParentBlock or Block copies.\n";
    std::cout << alloc_calls << " Heap allocations.\n";
    std::cout << alloc_bytes << " Heap bytes allocated.\n";
    std::cout << "Cache hits: " << Parser.CacheHits << "\n";
    std::cout << "Cache misses: " << Parser.CacheMisses << "\n";    
    
}