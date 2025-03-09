#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "../src/hash_function.h"
#include <string>
#include <sstream>
#include <iomanip>

inline ByteVector stringToByteVector(const std::string& str) {
    return ByteVector(str.begin(), str.end());
}

inline std::string bytesToHexString(const ByteVector& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

#endif // TEST_UTILS_H
