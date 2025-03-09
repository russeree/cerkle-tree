#ifndef SMT_H
#define SMT_H

#include "hash_function.h"
#include "hash_sha256.h"
#include "proof.h"
#include <bitset>
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
            SmtContext<H> temp;
        }
        return ZERO_HASHES[level];
    }

    static const ByteVector& getNullHash() {
        if (!ZERO_HASHES_INITIALIZED) {
            SmtContext<H> temp;
        }
        // Return the hash at level 0 (the hash of an empty value)
        return ZERO_HASHES[0];
    }

    SmtContext(const H& hashFunction = H())
        : hashFunction_(hashFunction) {
        
        if (!ZERO_HASHES_INITIALIZED) {
            initializeZeroHashes();
            ZERO_HASHES_INITIALIZED = true;
        }
        
        // Initialize defaultValue_ to the hash of an empty ByteVector (the null hash)
        defaultValue_ = ZERO_HASHES[0];
        root_ = ZERO_HASHES[256];
    }
    

    ByteVector getRootHash() const {
        return root_;
    }
    
    std::string getRootHashString() const {
        return H::hashToString(root_);
    }

    /**
     * @brief Set a leaf hash in the tree (directly stores the provided hash)
     * 
     * @param key The 256-bit key for the leaf position
     * @param hash The hash value to store at this leaf
     */
    void setLeafHash(const uint256_t& key, const ByteVector& hash) {
        setLeafNoUpdate(key, hash);
        updateMerkleRoot();
    }
    
    /**
     * @brief Set a leaf value in the tree (hashes the value first)
     * 
     * @param key The 256-bit key for the leaf position
     * @param value The value to hash and store at this leaf
     */
    void setLeafValue(const uint256_t& key, const ByteVector& value) {
        ByteVector hash = hashFunction_.hash(value);
        setLeafNoUpdate(key, hash);
        updateMerkleRoot();
    }
    
    /**
     * @brief Set a leaf value in the tree (legacy method, now calls setLeafHash)
     * 
     * @param key The 256-bit key for the leaf position
     * @param value The hash value to store at this leaf
     */
    void setLeaf(const uint256_t& key, const ByteVector& value) {
        setLeafValue(key, value);
    }

    /**
     * @brief Remove a leaf from the tree
     * 
     * @param key The 256-bit key of the leaf to remove
     */
    void removeLeaf(const uint256_t& key) {
        leaves_.erase(key);
        updateMerkleRoot();
    }

    /**
     * @brief Set multiple leaves in a batch by hash (direct), updating the root only once
     * 
     * @param updates Vector of key-value pairs to update
     */
    void setBatchLeavesHash(const std::vector<std::pair<uint256_t, ByteVector>>& updates) {
        if (updates.empty()) return;
        for (const auto& [key, value] : updates) {
            setLeafNoUpdate(key, value);
        }
        updateMerkleRoot();
    }

    /**
     * @brief Set multiple leaves in a batch by value, updating the root only once
     * 
     * @param updates Vector of key-value pairs to update
     */
    void setBatchLeavesValue(const std::vector<std::pair<uint256_t, ByteVector>>& updates) {
        if (updates.empty()) return;
        for (const auto& [key, value] : updates) {
            ByteVector hash = hashFunction_.hash(value);
            setLeafNoUpdate(key, hash);
        }
        updateMerkleRoot();
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
        updateMerkleRoot();
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
        std::map<uint256_t, ByteVector> processed;
        bool found = true; // Track if we can find the parent node
        
        // For each level from leaf to root
        for (int depth = 0; depth < 256; depth++) {
            uint256_t siblingKey = getPairedNode(currentKey);
            ByteVector siblingHash;
            
            if (depth == 0) {
                auto it = leaves_.find(siblingKey);
                if (it != leaves_.end()) {
                    siblingHash = it->second;
                    processed[siblingKey] = siblingHash;
                } else {
                    siblingHash = ZERO_HASHES[0];
                }
                processed = leaves_;
            } else {
                // Update update the depth of the tree moving the D -> (D + 1)
                processed = create_depth(processed, depth);

                auto processedIt = processed.find(siblingKey);
                if (processedIt != processed.end()) {
                    siblingHash = processedIt->second;
                } else {
                    siblingHash = ZERO_HASHES[depth];
                }
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

    /**
     * @brief Create a new map of leaves at a higher depth by hashing pairs of leaves
     * 
     * @param leaves The input map of leaves
     * @param level The current level in the tree
     * @return std::map<uint256_t, ByteVector> The new map of leaves at the next level
     */
    std::map<uint256_t, ByteVector> create_depth(std::map<uint256_t, ByteVector>& leaves, int level) const {
        // Create a new map to store the new leaves
        std::map<uint256_t, ByteVector> n_leaves;

        // Iterate over all leaves in the input map
        for (const auto& [key, value] : leaves) {
            // Check if this is a left node (last bit is 0)
            bool is_left = isLeft(key);
            
            // Get the paired node key by flipping the last bit
            uint256_t paired_key = getPairedNode(key);
            
            // Find the paired node in the input map
            auto paired_it = leaves.find(paired_key);
            
            // Get the hash of the paired node or use zero hash if not found
            ByteVector paired_hash;
            if (paired_it != leaves.end()) {
                paired_hash = paired_it->second;
            } else {
                // Use the zero hash for the current level
                // The zero hashes array is indexed with leaves at 0 and root at 256
                paired_hash = ZERO_HASHES[level];
            }
            
            // Combine the hashes in the correct order (left then right)
            ByteVector combined;
            if (is_left) {
                combined.insert(combined.end(), value.begin(), value.end());
                combined.insert(combined.end(), paired_hash.begin(), paired_hash.end());
            } else {
                combined.insert(combined.end(), paired_hash.begin(), paired_hash.end());
                combined.insert(combined.end(), value.begin(), value.end());
            }
            
            // Hash the combined value
            ByteVector new_hash = hashFunction_.hash(combined);
            
            // Right shift the key to get the parent key
            uint256_t new_key = key >> 1;
            
            // Insert the new key-hash pair into the new map
            // Only insert if we haven't processed this pair already
            if (n_leaves.find(new_key) == n_leaves.end()) {
                n_leaves[new_key] = new_hash;
            }
        }
        
        return n_leaves;
    }

    bool validateNonInclusionProof(const uint256_t& key, const MerkleProof& proof) const {
        if (!proof.isValid() || proof.size() != 256) {
            return false;
        }

        ByteVector currentHash = getNullHash();
        return validateProofPath(key, currentHash, proof);
    }

    bool validateProof(const uint256_t& key, const ByteVector& value, const MerkleProof& proof) const {
        if (!proof.isValid() || proof.size() != 256) {
            return false;
        }

        ByteVector currentHash = hashFunction_.hash(value);
        return validateProofPath(key, currentHash, proof);
    }

    /**
     * @brief Calculate the root hash by recursively applying create_depth
     * 
     * @param leaves The initial map of leaves
     * @return ByteVector The calculated root hash
     */
    ByteVector calculateRootFromLeaves(std::map<uint256_t, ByteVector>& leaves) const {
        // If the map is empty, return the zero hash for level 256
        if (leaves.empty()) {
            return ZERO_HASHES[256];
        }

        // Make a copy of the leaves to work with
        std::map<uint256_t, ByteVector> current_leaves = leaves;
        
        // For each leaf, ensure it has the correct value
        for (auto& [key, value] : current_leaves) {
            if (value.empty()) {
                value = defaultValue_;
            }
        }
        
        // Process all levels from 0 up to 255 using the create_depth method
        std::map<uint256_t, ByteVector> processed_leaves = current_leaves;
        for (int i = 0; i <= 255; i++) {
            processed_leaves = create_depth(processed_leaves, i);
            
            // If we've reached a single node, we can potentially stop early
            if (processed_leaves.size() == 1 && i == 255) {
                break;
            }
        }
        
        // If we have a single node at the end, that's our root
        if (processed_leaves.size() == 1) {
            ByteVector root = processed_leaves.begin()->second;
            return root;
        }
        
        // If we somehow didn't reach a single node (shouldn't happen), return the zero hash
        return ZERO_HASHES[256];
    }

    /**
     * @brief Calculate the root hash from the tree's current leaves using the create_depth method
     * 
     * @return ByteVector The calculated root hash
     */
    ByteVector calculateRootFromCurrentLeaves() const {
        // Create a copy of the current leaves
        std::map<uint256_t, ByteVector> leaves_copy = leaves_;
        
        // Use the calculateRootFromLeaves function to get the root
        return calculateRootFromLeaves(leaves_copy);
    }

private:
    bool validateProofPath(const uint256_t& key, ByteVector currentHash, const MerkleProof& proof) const {
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
        return currentHash == root_;
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
     */
    void updateMerkleRoot() {
        // If tree is empty after updates, use the initial root
        if (leaves_.empty()) {
            root_ = ZERO_HASHES[256];
            return;
        }
        
        // Otherwise calculate root from remaining leaves using the create_depth method
        root_ = calculateRootFromCurrentLeaves();
    }
};

template <typename H>
std::vector<ByteVector> SmtContext<H>::ZERO_HASHES;

template <typename H>
bool SmtContext<H>::ZERO_HASHES_INITIALIZED = false;

#endif // SMT_H
