#include "proof.h"
#include <stdexcept>

MerkleProof::MerkleProof() : valid_(false) {}

MerkleProof::MerkleProof(const std::vector<ByteVector>& siblings)
    : siblings_(siblings), valid_(true) {}

void MerkleProof::addSibling(const ByteVector& sibling) {
    siblings_.push_back(sibling);
    valid_ = true;
}

const ByteVector& MerkleProof::getSibling(size_t level) const {
    if (level >= siblings_.size()) {
        throw std::out_of_range("Level out of range");
    }
    return siblings_[level];
}

const std::vector<ByteVector>& MerkleProof::getSiblings() const {
    return siblings_;
}

bool MerkleProof::isValid() const {
    return valid_;
}

size_t MerkleProof::size() const {
    return siblings_.size();
}
