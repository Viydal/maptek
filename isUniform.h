#include <cstddef>

bool is_uniform(const char* block, size_t size) {
    char first = block[0];
    for (size_t i = 1; i < size; i++) {
        if (block[i] != first) return false;
    }
    return true;
}
