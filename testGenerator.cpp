#include "Test.h"
#include <iostream>

int main() {
    // Create Test object with just dimensions
    Test t("50000,2000,5,500,20,1" );

    // Generate the randomized test case
    t.MakeTest();

    // Print full test case
    std::cout << "Generated Test Case\n";
    for (const auto &line : t.testCase) {
        std::cout << line << "\n";
    }

    t.saveTestCase();

    return 0;
}