#include "hash_sha256.h"

ByteVector Sha256HashFunction::hash(const ByteVector& data) const {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, EVP_sha256(), NULL);
    EVP_DigestUpdate(context, data.data(), data.size());
    EVP_DigestFinal_ex(context, hash, &lengthOfHash);
    EVP_MD_CTX_free(context);
    ByteVector result(hash, hash + lengthOfHash);
    return result;
}
