#ifndef SMT_H
#define SMT_H

#include "hash_function.h"
#include "hash_sha256.h"
#include "proof.h"
#include <iostream>
#include <vector>
#include <bitset>
#include <map>
#include <boost/multiprecision/cpp_int.hpp>

using uint256_t = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

template <typename H = Sha256HashFunction>
class SmtContext {
private:
    static std::vector<ByteVector> ZERO_HASHES;
    static bool ZERO_HASHES_INITIALIZED;
public:
    static const ByteVector& getZeroHash(size_t level) {
        if (!ZERO_HASHES_INITIALIZED) {
            SmtContext<H> temp;  // This will initialize ZERO_HASHES
        }
        return ZERO_HASHES[level];
    }

    /**
     * @brief Construct a new SmtContext object
     * 
     * @param hashFunction The hash function to use (default constructed if not provided)
     */
    SmtContext(const H& hashFunction = H())
        : defaultValue_(), hashFunction_(hashFunction) {
        
        if (!ZERO_HASHES_INITIALIZED) {
            initializeZeroHashes();
            ZERO_HASHES_INITIALIZED = true;
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
        setLeafNoUpdate(key, value);
        updateMerkleRoot(key);
    }

    /**
     * @brief Remove a leaf from the tree
     * 
     * @param key The 256-bit key of the leaf to remove
     */
    void removeLeaf(const uint256_t& key) {
        leaves_.erase(key);
        updateMerkleRoot(key);
    }

    /**
     * @brief Set multiple leaves in a batch, updating the root only once
     * 
     * @param updates Vector of key-value pairs to update
     */
    void setBatchLeaves(const std::vector<std::pair<uint256_t, ByteVector>>& updates) {
        if (updates.empty()) return;
        for (const auto& [key, value] : updates) {
            setLeafNoUpdate(key, value);
        }
        updateMerkleRoot(updates.back().first);
    }

    /**
     * @brief Remove multiple leaves in a batch
     * 
     * @param keys Vector of keys to remove
     */
    void removeBatchLeaves(const std::vector<uint256_t>& keys) {
        if (keys.empty()) return;
        for (const auto& key : keys) {
            leaves_.erase(key);
        }
        updateMerkleRoot(keys.back());
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


    MerkleProof generateProof(const uint256_t& key) const {
        MerkleProof proof;
        uint256_t currentKey = key;
        
        // Collect sibling hashes from leaf to root
        for (int level = 256; level > 0; level--) {
            uint256_t siblingKey = getPairedNode(currentKey);
            ByteVector siblingHash;
            
            // Get sibling's hash (either from tree or zero hash)
            uint256_t siblingSubtreeStart = siblingKey << (256 - level);
            uint256_t siblingSubtreeEnd = (siblingKey + 1) << (256 - level);
            auto it = leaves_.lower_bound(siblingSubtreeStart);
            if (it != leaves_.end() && it->first < siblingSubtreeEnd) {
                siblingHash = calculateNodeHash(siblingKey, level);
            } else {
                siblingHash = ZERO_HASHES[level];
            }
            
            proof.addSibling(siblingHash);
            currentKey >>= 1;
        }
        
        return proof;
    }

    /**
     * @brief Clear all leaves from the tree and update the root
     */
    void clearLeaves() {
        leaves_.clear();
        root_ = ZERO_HASHES[256];
    }

    bool validateProof(const uint256_t& key, const ByteVector& value, const MerkleProof& proof) const {
        if (!proof.isValid() || proof.size() != 256) {
            return false;
        }

        ByteVector currentHash = value;
        uint256_t currentKey = key;
        
        // Reconstruct path from leaf to root
        for (int level = 256; level > 0; level--) {
            ByteVector combined;
            if (isLeft(currentKey)) {
                combined.insert(combined.end(), currentHash.begin(), currentHash.end());
                combined.insert(combined.end(), proof.getSibling(256-level).begin(), proof.getSibling(256-level).end());
            } else {
                combined.insert(combined.end(), proof.getSibling(256-level).begin(), proof.getSibling(256-level).end());
                combined.insert(combined.end(), currentHash.begin(), currentHash.end());
            }
            
            currentHash = hashFunction_.hash(combined);
            currentKey >>= 1;
        }
        
        // For empty tree or when all leaves have default values, compare against ZERO_HASHES[256]
        return currentHash == (leaves_.empty() ? ZERO_HASHES[256] : root_);
    }

private:
    ByteVector defaultValue_;
    ByteVector root_;
    H hashFunction_;
    std::map<uint256_t, ByteVector> leaves_;

    void setLeafNoUpdate(const uint256_t& key, const ByteVector& value) {
        if (value == defaultValue_) {
            leaves_.erase(key);
        } else {
            leaves_[key] = value;
        }
    }
    
    void initializeZeroHashes() {
        ZERO_HASHES.resize(257);

        // We are going to use the null hashes to represent empties.
        ZERO_HASHES[0] = hashFunction_.hash(ByteVector());
        
        for (size_t i = 1; i <= 256; i++) {
            ByteVector combined;
            combined.insert(combined.end(), ZERO_HASHES[i-1].begin(), ZERO_HASHES[i-1].end());
            combined.insert(combined.end(), ZERO_HASHES[i-1].begin(), ZERO_HASHES[i-1].end());
            ZERO_HASHES[i] = hashFunction_.hash(combined);
        }
    }

    /**
     * @brief Calculate the hash for a node at a given level
     * 
     * @param key The node's key
     * @param level The level in the tree (0 to 256)
     * @return ByteVector The hash for this node
     */
    ByteVector calculateNodeHash(uint256_t key, int level) const {
        if (level == 256) {
            return getLeaf(key);
        }

        // Get left and right child keys
        uint256_t leftKey = key << 1;
        uint256_t rightKey = (key << 1) | 1;
        
        // Get left child hash
        ByteVector leftHash;
        uint256_t leftSubtreeStart = leftKey << (256 - (level + 1));
        uint256_t leftSubtreeEnd = (leftKey + 1) << (256 - (level + 1));
        auto leftIt = leaves_.lower_bound(leftSubtreeStart);
        if (leftIt != leaves_.end() && leftIt->first < leftSubtreeEnd) {
            leftHash = calculateNodeHash(leftKey, level + 1);
        } else {
            leftHash = ZERO_HASHES[level + 1];
        }
        
        // Get right child hash
        ByteVector rightHash;
        uint256_t rightSubtreeStart = rightKey << (256 - (level + 1));
        uint256_t rightSubtreeEnd = (rightKey + 1) << (256 - (level + 1));
        auto rightIt = leaves_.lower_bound(rightSubtreeStart);
        if (rightIt != leaves_.end() && rightIt->first < rightSubtreeEnd) {
            rightHash = calculateNodeHash(rightKey, level + 1);
        } else {
            rightHash = ZERO_HASHES[level + 1];
        }
        
        // Combine and hash
        ByteVector combined;
        combined.insert(combined.end(), leftHash.begin(), leftHash.end());
        combined.insert(combined.end(), rightHash.begin(), rightHash.end());
        return hashFunction_.hash(combined);
    }

    /**
     * @brief Update the merkle root after a leaf change
     * 
     * @param key The 256-bit key of the modified leaf
     */
    void updateMerkleRoot(uint256_t) {
        // If tree is empty after updates, use the initial root
        if (leaves_.empty()) {
            root_ = ZERO_HASHES[256];
            return;
        }
        
        // Otherwise calculate root from remaining leaves
        root_ = calculateNodeHash(0, 0);
    }
};

template <typename H>
std::vector<ByteVector> SmtContext<H>::ZERO_HASHES;

template <typename H>
bool SmtContext<H>::ZERO_HASHES_INITIALIZED = false;

#endif // SMT_H
