#ifndef SMT_H
#define SMT_H

#include "hash_function.h"
#include "hash_sha256.h"
#include <iostream>
#include <vector>
#include <bitset>

extern std::vector<ByteVector> ZERO_HASHES;

template <typename H = Sha256HashFunction>
class SmtContext {
public:
    /**
     * @brief Construct a new SmtContext object
     * 
     * @param defaultValue The default value for empty leaves
     * @param hashFunction The hash function to use (default constructed if not provided)
     */
    SmtContext(const ByteVector& defaultValue, const H& hashFunction = H())
        : defaultValue_(defaultValue), hashFunction_(hashFunction) {
        // Initialize zero hashes if not already initialized
        if (ZERO_HASHES.empty()) {
            initializeZeroHashes();
        }
        
        // Initialize the root with the zero hash at depth 256
        root_ = ZERO_HASHES[256];
    }
    

    ByteVector getRootHash() const {
        return root_;
    }
    
    std::string getRootHashString() const {
        return H::hashToString(root_);
    }

private:
    ByteVector defaultValue_;
    ByteVector root_;
    H hashFunction_;
    
    void initializeZeroHashes() {
        ZERO_HASHES.resize(257);
        // We are going to use the null hashes to represent empties.
        ZERO_HASHES[0] = hashFunction_.hash(ByteVector());
        
        std::cout << "Initializing Zero Hashes:\n";
        std::cout << "Level 0 (Null Hash): " << H::hashToString(ZERO_HASHES[0]) << "\n";
        
        for (size_t i = 1; i <= 256; i++) {
            ByteVector combined;
            combined.insert(combined.end(), ZERO_HASHES[i-1].begin(), ZERO_HASHES[i-1].end());
            combined.insert(combined.end(), ZERO_HASHES[i-1].begin(), ZERO_HASHES[i-1].end());
            ZERO_HASHES[i] = hashFunction_.hash(combined);
            
            std::cout << "Level " << i << ": " << H::hashToString(ZERO_HASHES[i]) << "\n";
        }
    }
};

#endif // SMT_H
