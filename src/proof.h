#ifndef MERKLE_PROOF_H
#define MERKLE_PROOF_H

#include "hash_function.h"
#include <vector>

class MerkleProof {
public:
    MerkleProof();
    explicit MerkleProof(const std::vector<ByteVector>& siblings);

    // Add sibling hash to the proof
    void addSibling(const ByteVector& sibling);
    
    const ByteVector& getSibling(size_t level) const;
    const std::vector<ByteVector>& getSiblings() const;
    bool isValid() const;
    size_t size() const;

private:
    std::vector<ByteVector> siblings_;  // Sibling hashes along the path
    bool valid_;                        // Generated successfully
};

#endif // MERKLE_PROOF_H
