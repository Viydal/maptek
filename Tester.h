#ifndef TESTER_H
#define TESTER_H

// Not to be Confused w/ test.h this handles: central hub for all testing, including setup and result reporting.


#include "Parse.h"
#include "Compression.h"
#include "Test.h"
#include <string>

class Tester {
public:
    static bool RunTest(const std::string& filePath, bool verbose = false, int verboseLevel = 1);
    static void RunAllTests(bool verbose = false, int verboseLevel = 1);
};

#endif
