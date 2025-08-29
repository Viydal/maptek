#include "Parse.h"
#include "Compression.h"
#include "Test.h"
#include <iostream>
#include <vector>
#include <sstream>

int main() {
    // --- Hardcoded input map (RLE rows) ---
    std::vector<std::string> inputLines = {
        "16,6,2,4,3,1",      // header
        "o, sea",
        "w, WA",
        "n, NT",
        "s, SA",
        "q, QLD",
        "e, NSW",
        "v, VIC",
        "t, TAS",
        "",
        "eeeooooooooooooo",
        "owwwnnnneeeqqqqo",
        "owwwnnnneeeqqqqo",
        "osssvvvvttttoooo",
        "osssvvvvttttoooo",
        "ssoooooooooooooo",
        "",
        "oooooooooooooooo",
        "wwwwwwwwwwwwwwww",
        "ooosssvvvttttooo",
        "ooosssvvvttttooo",
        "oooooooooooooooo",
        "oooooooooooooooo"
    };

    // --- Hardcoded output rectangles (with one wrong value) ---
    std::vector<std::string> expectedOutputLines = {
        "0,0,0,3,1,1,NSW",
        "3,0,0,1,1,1,sea",
        "4,0,0,4,1,1,sea",
        "8,0,0,4,1,1,sea",
        "12,0,0,4,1,1,sea",
        "0,1,0,1,2,1,sea",
        "1,1,0,3,2,1,WA",
        "4,1,0,4,2,1,NT",
        "8,1,0,3,2,1,NSW",
        "11,1,0,1,2,1,QLD",
        "12,1,0,3,2,1,QLD",
        "15,1,0,1,2,1,sea",
        "0,3,0,1,2,1,sea",
        "1,3,0,3,2,1,SA",
        "4,3,0,4,2,1,VIC",
        "8,3,0,4,2,1,TAS",
        "12,3,0,4,3,1,sea",
        "0,5,0,2,1,1,SA",
        "2,5,0,2,1,1,sea",
        "4,5,0,4,1,1,sea",
        "8,5,0,4,1,1,sea",

        // Layer 1 (introduce a mismatch in the first rectangle)
        "0,0,1,4,1,1,sea",
        "4,0,1,4,1,1,sea",
        "8,0,1,4,1,1,sea",
        "12,0,1,4,1,1,**WA**",   // <-- changed from 'sea' to 'WA' (wrong)
        "0,1,1,4,1,1,WA",
        "4,1,1,4,1,1,WA",
        "8,1,1,4,1,1,WA",
        "12,1,1,4,1,1,WA",
        "0,2,1,3,1,1,sea",
        "3,2,1,1,1,1,SA",
        "4,2,1,2,1,1,SA",
        "6,2,1,2,1,1,VIC",
        "8,2,1,1,1,1,VIC",
        "9,2,1,3,1,1,TAS",
        "12,2,1,1,1,1,TAS",
        "13,2,1,3,1,1,sea",
        "0,3,1,3,1,1,sea",
        "3,3,1,1,1,1,SA",
        "4,3,1,2,1,1,SA",
        "6,3,1,2,1,1,VIC",
        "8,3,1,1,1,1,VIC",
        "9,3,1,3,1,1,TAS",
        "12,3,1,1,1,1,TAS",
        "13,3,1,3,1,1,sea",
        "0,4,1,4,2,1,sea",
        "4,4,1,4,2,1,sea",
        "8,4,1,4,2,1,sea",
        "12,4,1,4,2,1,sea"
    };

    // --- Create Test object and reconstruct OutputParse ---
    Test myTest(inputLines, expectedOutputLines);

    // --- Print maps ---
    myTest.printInputParse();
    myTest.printOutputParse();

    // --- Compare input vs reconstructed output ---
    if (myTest.compareInputOutput()) {
        std::cout << "TEST PASSED: Output matches input.\n";
    } else {
        std::cout << "TEST FAILED: Output does not match input.\n";
    }

    return 0;
}
