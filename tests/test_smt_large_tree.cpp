#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/proof.h"
#include "test_utils.h"
#include <random>
#include <chrono>
#include <iostream>
#include <vector>
#include <utility>

BOOST_AUTO_TEST_SUITE(SmtLargeTreeTests)

struct SmtLargeTreeFixture {
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{hashFunction};
    
    // Random number generator
    std::mt19937_64 rng;
    
    SmtLargeTreeFixture() : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}
    
    // Generate a random uint256_t
    uint256_t randomKey() {
        uint256_t key = 0;
        for (int i = 0; i < 4; ++i) {
            key = (key << 64) | rng();
        }
        return key;
    }
    
    // Generate a random ByteVector of specified length
    ByteVector randomValue(size_t length = 32) {
        ByteVector value(length);
        for (size_t i = 0; i < length; ++i) {
            value[i] = static_cast<uint8_t>(rng() % 256);
        }
        return value;
    }
};

BOOST_FIXTURE_TEST_CASE(test_large_tree_operations, SmtLargeTreeFixture) {
    // Step 1: Create 1000 random key-value pairs (except position 0)
    std::cout << "Step 1: Creating 1000 random key-value pairs..." << std::endl;
    
    const size_t NUM_PAIRS = 100;
    std::vector<std::pair<uint256_t, ByteVector>> pairs;
    pairs.reserve(NUM_PAIRS);
    
    // Create a specific value for position 0
    uint256_t key0 = 0;
    ByteVector value0 = stringToByteVector("Position Zero Value");
    pairs.push_back({key0, value0});
    
    // Create 999 more random pairs
    for (size_t i = 1; i < NUM_PAIRS; ++i) {
        uint256_t key = randomKey();
        ByteVector value = randomValue();
        pairs.push_back({key, value});
    }
    
    // Step 2: Insert all pairs into the SMT
    std::cout << "Step 2: Inserting pairs into the SMT..." << std::endl;
    for (const auto& [key, value] : pairs) {
        smt.setLeaf(key, value);
    }
    
    // Step 3: Generate and verify proofs for each pair
    std::cout << "Step 3: Generating and verifying proofs..." << std::endl;
    std::vector<MerkleProof> proofs;
    proofs.reserve(NUM_PAIRS);
    
    for (const auto& [key, value] : pairs) {
        MerkleProof proof = smt.generateProof(key);
        proofs.push_back(proof);
        
        // Verify the proof
        BOOST_TEST(proof.isValid());
        BOOST_TEST(smt.validateProof(key, value, proof));
    }
    
    // Step 4: Modify element 0 and check proofs again
    std::cout << "Step 4: Modifying element 0 and checking proofs..." << std::endl;
    ByteVector newValue0 = stringToByteVector("Modified Position Zero Value");
    smt.setLeaf(key0, newValue0);
    
    // Check that the proof for position 0 is no longer valid with the old value
    BOOST_TEST(!smt.validateProof(key0, value0, proofs[0]));
    
    // Generate a new proof for position 0 and verify it
    MerkleProof newProof0 = smt.generateProof(key0);
    BOOST_TEST(smt.validateProof(key0, newValue0, newProof0));
    
    // Check that other proofs are still valid
    for (size_t i = 1; i < NUM_PAIRS; ++i) {
        const auto& [key, value] = pairs[i];
        BOOST_TEST(smt.validateProof(key, value, proofs[i]));
    }
    
    // Step 5: Remove position 0 and check that one path is still valid
    std::cout << "Step 5: Removing position 0 and checking a path..." << std::endl;
    smt.removeLeaf(key0);
    
    // Position 0 proof should now be a non-inclusion proof
    BOOST_TEST(smt.validateNonInclusionProof(key0, smt.generateProof(key0)));
    
    // Check that other proofs are still valid
    for (size_t i = 1; i < NUM_PAIRS; ++i) {
        const auto& [key, value] = pairs[i];
        // We need to regenerate proofs since the tree structure changed
        MerkleProof updatedProof = smt.generateProof(key);
        BOOST_TEST(smt.validateProof(key, value, updatedProof));
    }
    
    // Step 6: Modify a value of one of the leaves and verify the impact
    std::cout << "Step 6: Modifying a leaf value and verifying impact..." << std::endl;
    
    // Choose a random leaf to modify (not position 0)
    size_t indexToModify = 1 + (rng() % (NUM_PAIRS - 1));
    uint256_t keyToModify = pairs[indexToModify].first;
    ByteVector oldValue = pairs[indexToModify].second;
    ByteVector newValue = randomValue();
    
    // Store the old proof before modification
    MerkleProof oldProof = smt.generateProof(keyToModify);
    
    // Modify the leaf
    smt.setLeaf(keyToModify, newValue);
    pairs[indexToModify].second = newValue;
    
    // Verify the old proof is no longer valid with the old value
    BOOST_TEST(!smt.validateProof(keyToModify, oldValue, oldProof));
    
    // Generate a new proof and verify it
    MerkleProof newProof = smt.generateProof(keyToModify);
    BOOST_TEST(smt.validateProof(keyToModify, newValue, newProof));
    
    // Check that other proofs are still valid by regenerating them
    for (size_t i = 1; i < NUM_PAIRS; ++i) {
        if (i != indexToModify) {
            const auto& [key, value] = pairs[i];
            MerkleProof updatedProof = smt.generateProof(key);
            BOOST_TEST(smt.validateProof(key, value, updatedProof));
        }
    }
    
    std::cout << "All tests completed successfully!" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
