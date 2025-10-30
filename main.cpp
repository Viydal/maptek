#include "Tester.h"
#include "Compression.h"
#include "Parse.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdio>
#include <omp.h>

//chat gpt windows output speedup
#ifdef _WIN32
  #define NOMINMAX
  #include <io.h>    // _setmode, _fileno
  #include <fcntl.h> // _O_BINARY
#endif


int main(int argc, char* argv[]) {

std::ios::sync_with_stdio(false);
std::cin.tie(nullptr);
std::cout.tie(nullptr);

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

    // read in from file or stdin

    std::unique_ptr<std::ifstream> file;
    std::istream* in = &std::cin; 

    // liberal use of generative ai here to get this working, some special form of pointer is used
    if (Args.readFile) {
        
        file = std::make_unique<std::ifstream>("TestCases/" + Args.filePath);
        if (!*file) {
            std::cerr << "Error: could not open " << Args.filePath << "\n";
            return 1;
        }
        in = file.get();
    }
    
    Parse Parser;

    Parser.StreamParseHeader(*in);

    //create the parent blocks which will be constructed and sorted in place

    std::vector<ParentBlock> ParentBlocks;
    ParentBlocks.reserve(Parser.NumXBlocks * Parser.NumYBlocks);

    for (int BlockRow = 0; BlockRow < Parser.NumYBlocks; BlockRow++) {
        for (int BlockCol = 0; BlockCol < Parser.NumXBlocks; BlockCol++) {
            ParentBlocks.emplace_back(BlockCol * Parser.ParentX, BlockRow * Parser.ParentY, 0, Parser.ParentX, Parser.ParentY, Parser.ParentZ);
        }
    }

    Compression Compressor;
    std::string Output;
    Output.reserve(ParentBlocks.size() * 80);
    //Processes a ParentX * ParentY * ParentZ chunk of the map at a time
    omp_set_num_threads(4);
    for (int i = 0; i < Parser.NumZBlocks; i++){
        Output.clear();
        #pragma omp parallel for schedule(static)
        for (int idx = 0; idx < ParentBlocks.size(); idx++) {
            ParentBlocks[idx].StartZ = i * Parser.ParentZ;
            ParentBlocks[idx].Blocks.clear();
        }

        Parser.StreamParseMapChunk(ParentBlocks, i, *in);
        size_t total = 0;
        std::vector<std::string> chunks(ParentBlocks.size());
        #pragma omp parallel for schedule(static)
        for (int idx = 0; idx < (int)ParentBlocks.size(); ++idx) {
            Compressor.CompressParentBlock(ParentBlocks[idx]);
            ParentBlocks[idx].WriteBlock(Parser.TagTable, chunks[idx]);
            total += chunks[idx].size();
        }
        Output.clear();
        Output.reserve(total);
        for (auto& s : chunks) Output.append(s);

        std::cout.write(Output.data(), Output.size());
    }   
}


