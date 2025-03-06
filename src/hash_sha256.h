#ifndef HASH_SHA256_H
#define HASH_SHA256_H

#include "hash_function.h"
#include <openssl/evp.h>

/**
 * @brief OpenSSL SHA256 implementation of the HashFunction interface
 * 
 * Note: Uses EVP interface which is the recommended approach in OpenSSL 3.0+
 */
class Sha256HashFunction : public HashFunction {
public:
    ByteVector hash(const ByteVector& data) const override;
};

#endif // HASH_SHA256_H
