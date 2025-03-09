#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/proof.h"
#include "test_utils.h"
#include <random>
#include <chrono>
#include <limits>
#include <iostream>

BOOST_AUTO_TEST_SUITE(SmtProofTests)

struct SmtProofFixture {
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{hashFunction};
};

BOOST_FIXTURE_TEST_CASE(test_proof_single_item, SmtProofFixture) {
    uint256_t key = 255;
    ByteVector value{};
    
    smt.setLeaf(key, value);
    MerkleProof proof = smt.generateProof(key);
    bool validationResult = smt.validateProof(key, value, proof);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    BOOST_TEST(validationResult);
}

BOOST_FIXTURE_TEST_CASE(test_proof_single_leaf, SmtProofFixture) {
    uint256_t key = 63;
    ByteVector value{0x01, 0x02, 0x03};
    
    smt.setLeaf(key, value);
    MerkleProof proof = smt.generateProof(key);
    
    bool validationResult = smt.validateProof(key, value, proof);
    BOOST_TEST(proof.isValid());
    BOOST_TEST(validationResult);
    
    ByteVector wrongValue{0x04, 0x05, 0x06};
    bool wrongValidationResult = smt.validateProof(key, wrongValue, proof);
    BOOST_TEST(!wrongValidationResult);
}

BOOST_FIXTURE_TEST_CASE(test_proof_multiple_leaves, SmtProofFixture) {
    uint256_t key1 = 0;
    uint256_t key2 = 2;
    ByteVector value1{0x01, 0x01, 0x01};
    ByteVector value2{0x02, 0x02, 0x02};

    Sha256HashFunction hashFunction;
    ByteVector hash1 = hashFunction.hash(value1);
    ByteVector hash2 = hashFunction.hash(value2);
    std::cout << "Original value1: " << bytesToHexString(value1) << std::endl;
    std::cout << "Original value2: " << bytesToHexString(value2) << std::endl;
    std::cout << "Hash of value1: " << bytesToHexString(hash1) << std::endl;
    std::cout << "Hash of value2: " << bytesToHexString(hash2) << std::endl;

    // Double set the same leaf.
    smt.setLeaf(key1, value1);
    smt.setLeaf(key1, value1);
    MerkleProof double_key_update_proof = smt.generateProof(key1);
    BOOST_TEST(smt.validateProof(key1, value1, double_key_update_proof));

    // Set the first leaf and test a proof
    smt.setLeaf(key1, value1);
    MerkleProof proof1 = smt.generateProof(key1);
    BOOST_TEST(smt.validateProof(key1, value1, proof1));

    // Set the second leaf and check the proof again
    smt.setLeaf(key1, value1);
    MerkleProof proof2 = smt.generateProof(key1);
    BOOST_TEST(smt.validateProof(key1, value1, proof2));
    
    // Generate and validate proof for key2
    smt.setLeaf(key2, value2);
    MerkleProof proof3 = smt.generateProof(key2);
    BOOST_TEST(smt.validateProof(key2, value2, proof3));
    
    // Cross-validation tests
    bool crossValidation1 = smt.validateProof(key1, value2, proof1);
    BOOST_TEST(!crossValidation1);
    
    bool crossValidation2 = smt.validateProof(key2, value1, proof2);
    BOOST_TEST(!crossValidation2);
    
    bool crossValidation3 = smt.validateProof(key2, value2, proof1);
    BOOST_TEST(!crossValidation3);
    
    bool crossValidation4 = smt.validateProof(key1, value1, proof2);
    BOOST_TEST(!crossValidation4);
}

BOOST_FIXTURE_TEST_CASE(test_proof_null_hash_at_index_zero, SmtProofFixture) {
    // Create an empty SMT (no leaves added)
    ByteVector nullBytes = {};
    ByteVector rootHash = smt.getRootHash();
    
    // Generate a proof for key 0
    uint256_t key = 0;
    MerkleProof proof = smt.generateProof(key);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    
    // Verify that the proof validates correctly with the null hash
    bool validationResult = smt.validateProof(key, {}, proof);
    BOOST_TEST(validationResult);
}

BOOST_FIXTURE_TEST_CASE(test_proof_performance_with_random_values, SmtProofFixture) {
    std::cout << "Starting test_proof_performance_with_random_values" << std::endl;
    const size_t TEST_LEAVES_COUNT = 1000;
    
    // Vector to store the key-value pairs for testing
    std::vector<std::pair<uint256_t, ByteVector>> test_leaves;
    test_leaves.reserve(TEST_LEAVES_COUNT);
    
    // Create random generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> key_dist(0, std::numeric_limits<uint64_t>::max());
    std::uniform_int_distribution<int> value_size_dist(1, 32); // Random value size between 1 and 32 bytes
    std::uniform_int_distribution<uint8_t> byte_dist(0, 255);
    
    // Generate random leaves and add to SMT
    std::cout << "Generating " << TEST_LEAVES_COUNT << " random leaves and adding to SMT..." << std::endl;
    auto start_insertion = std::chrono::high_resolution_clock::now();
    
    // Generate all leaves first
    for (size_t i = 0; i < TEST_LEAVES_COUNT; i++) {
        // Generate random key (using only 64 bits for simplicity)
        uint256_t key = key_dist(gen);
        
        // Generate random value with random size
        int value_size = value_size_dist(gen);
        ByteVector value;
        value.reserve(value_size);
        for (int j = 0; j < value_size; j++) {
            value.push_back(byte_dist(gen));
        }
        
        // Store for later insertion and verification
        test_leaves.emplace_back(key, value);
    }
    
    // Use batch insert instead of individual inserts
    smt.setBatchLeavesValue(test_leaves);
    
    // Log the batch insertion
    std::cout << "  Added " << TEST_LEAVES_COUNT << " leaves using batch insert" << std::endl;
    
    auto end_insertion = std::chrono::high_resolution_clock::now();
    auto insertion_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_insertion - start_insertion).count();
    std::cout << "Insertion of " << TEST_LEAVES_COUNT << " leaves completed in " 
              << insertion_duration << " ms" << std::endl;
    
    // Get the root hash
    ByteVector root_hash = smt.getRootHash();
    std::cout << "Root hash: " << bytesToHexString(root_hash) << std::endl;
    
    // Generate and verify proofs for all leaves
    std::cout << "Generating and verifying proofs for all " << TEST_LEAVES_COUNT << " leaves..." << std::endl;
    auto start_proof = std::chrono::high_resolution_clock::now();
    
    size_t successful_proofs = 0;
    for (size_t i = 0; i < test_leaves.size(); i++) {
        const auto& [key, value] = test_leaves[i];
        
        // Generate proof
        MerkleProof proof = smt.generateProof(key);
        
        // Verify proof
        bool is_valid = smt.validateProof(key, value, proof);
        
        if (is_valid) {
            successful_proofs++;
        } else {
            std::cout << "  Proof validation failed for key: " << key << std::endl;
        }
        
        // Log progress every 100 proofs
        if (i % 100 == 0 && i > 0) {
            std::cout << "  Verified " << i << " proofs so far..." << std::endl;
        }
    }
    
    auto end_proof = std::chrono::high_resolution_clock::now();
    auto proof_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_proof - start_proof).count();
    
    std::cout << "Proof generation and verification completed in " << proof_duration << " ms" << std::endl;
    std::cout << "Successfully verified " << successful_proofs << " out of " 
              << TEST_LEAVES_COUNT << " proofs" << std::endl;
    
    // Verify all proofs were successful
    BOOST_TEST(successful_proofs == TEST_LEAVES_COUNT);
    
    // Test a non-existent key
    std::cout << "Testing proof for a non-existent key..." << std::endl;
    uint256_t non_existent_key = 0;
    bool found = false;
    
    // Find a key that doesn't exist in our test set
    while (!found) {
        non_existent_key = key_dist(gen);
        found = true;
        for (const auto& [key, _] : test_leaves) {
            if (key == non_existent_key) {
                found = false;
                break;
            }
        }
    }
    
    // Generate and verify proof for non-existent key
    MerkleProof non_existent_proof = smt.generateProof(non_existent_key);
    bool non_existent_valid = smt.validateProof(non_existent_key, ByteVector{}, non_existent_proof);
    
    std::cout << "Non-existent key proof validation result: " 
              << (non_existent_valid ? "Valid (expected for empty value)" : "Invalid") << std::endl;
    
    // The proof should be valid for an empty value if the key doesn't exist
    BOOST_TEST(non_existent_valid);
    
    std::cout << "test_proof_performance_with_random_values completed" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
