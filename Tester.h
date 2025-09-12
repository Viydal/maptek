#ifndef TESTER_H
#define TESTER_H

// Not to be Confused w/ test.h this handles: central hub for all testing, including setup and result reporting.


#include "Parse.h"
#include "Compression.h"
#include "Test.h"
#include "Helpers.h"
#include <string>

class Tester {
public:
    static bool RunTest(Args args);
    static void RunAllTests(Args args);
};
#endif
