#ifndef SMT_H
#define SMT_H

#include "hash_function.h"
#include "hash_sha256.h"
#include <iostream>
#include <vector>
#include <bitset>
#include <unordered_map>
#include <boost/multiprecision/cpp_int.hpp>

using uint256_t = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

// Custom hash function for uint256_t to be used in unordered_map
namespace std {
    template <>
    struct hash<uint256_t> {
        size_t operator()(const uint256_t& x) const {
            return static_cast<size_t>(x & std::numeric_limits<size_t>::max());
        }
    };
}

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
        
        if (ZERO_HASHES.empty()) {
            initializeZeroHashes();
        }
        
        root_ = ZERO_HASHES[256];
    }
    

    ByteVector getRootHash() const {
        return root_;
    }
    
    std::string getRootHashString() const {
        return H::hashToString(root_);
    }

    /**
     * @brief Set a leaf value in the tree
     * 
     * @param key The 256-bit key for the leaf position
     * @param value The hash value to store at this leaf
     */
    void setLeaf(const uint256_t& key, const ByteVector& value) {
        leaves_[key] = value;
        // !!!FIXME!!! Needs to add in the Update Function
    }

    /**
     * @brief Get a leaf value from the tree
     * 
     * @param key The 256-bit key for the leaf position
     * @return ByteVector The stored hash value, or defaultValue_ if not found
     */
    ByteVector getLeaf(const uint256_t& key) const {
        auto it = leaves_.find(key);
        return (it != leaves_.end()) ? it->second : defaultValue_;
    }

    /**
     * @brief Check if a leaf exists in the tree
     * 
     * @param key The 256-bit key to check
     * @return true if the leaf exists, false otherwise
     */
    bool hasLeaf(const uint256_t& key) const {
        return leaves_.find(key) != leaves_.end();
    }

    /**
     * @brief Check if a node is a left node (last bit is 0)
     * 
     * @param key The 256-bit key to check
     * @return true if the node is a left node (last bit is 0), false otherwise
     */
    bool isLeft(const uint256_t& key) const {
        return (key & 1) == 0;
    }

    /**
     * @brief Get the paired node's key by flipping the last bit
     * Left (last bit 0), returns right pair (last bit 1)
     * Right (last bit 1), returns left pair (last bit 0)
     * 
     * @param key The 256-bit key to get the pair for
     * @return uint256_t The key of the paired node
     */
    uint256_t getPairedNode(const uint256_t& key) const {
        return isLeft(key) ? (key | 1) : (key & ~uint256_t(1));
    }

private:
    ByteVector defaultValue_;
    ByteVector root_;
    H hashFunction_;
    std::unordered_map<uint256_t, ByteVector> leaves_;
    
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
