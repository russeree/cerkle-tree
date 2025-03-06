#ifndef HASH_FUNCTION_H
#define HASH_FUNCTION_H

#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>

using ByteVector = std::vector<uint8_t>;

/**
 * @brief SMT Hash function interface
 * 
 * Acts as an interface between a hashing function (shs256, RIPEMD, etc...) and our 
 * Implementations must provide a method to hash a byte vector and return the result as a byte vector.
 */
class HashFunction {
public:
    virtual ~HashFunction() = default;
    
    virtual ByteVector hash(const ByteVector& data) const = 0;
    static std::string hashToString(const ByteVector& hash);
};

#endif // HASH_FUNCTION_H
