#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "../src/hash_function.h"
#include <string>

inline ByteVector stringToByteVector(const std::string& str) {
    return ByteVector(str.begin(), str.end());
}

#endif // TEST_UTILS_H
