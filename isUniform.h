#ifndef ISUNIFORM_H
#define ISUNIFORM_H
#include <cstddef>
#include <string>
/*bool is_uniform(const char* block, size_t size) {
    char first = block[0];
    for (size_t i = 1; i < size; i++) {
        if (block[i] != first) return false;
    }
    return true;
}*/

bool is_uniform(std::string block) {
    char first = block[0];
    for (size_t i = 1; i < block.size(); i++) {
        if (block[i] != first) return false;
    }
    return true;
}
#endif