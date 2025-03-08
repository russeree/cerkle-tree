#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/hash_sha256.h"
#include "test_utils.h"
#include <chrono>

BOOST_AUTO_TEST_SUITE(SmtRootCalculationTests)

struct SmtRootCalcFixture {
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{hashFunction};
};

BOOST_FIXTURE_TEST_CASE(test_create_depth, SmtRootCalcFixture) {
    BOOST_TEST_MESSAGE("Testing create_depth function");
    
    // Create a map with two leaves at level 256
    std::map<uint256_t, ByteVector> leaves;
    ByteVector value1{0x01, 0x01, 0x01};
    ByteVector value2{0x02, 0x02, 0x02};
    
    // Add two leaves with paired keys (0 and 1)
    leaves[0] = value1;
    leaves[1] = value2;
    
    // Apply create_depth to get level 255
    std::map<uint256_t, ByteVector> level255 = smt.create_depth(leaves, 256);
    
    // Verify we have one node at level 255
    BOOST_TEST(level255.size() == 1);
    BOOST_TEST(level255.begin()->first == 0);
    
    // Manually calculate the expected hash
    ByteVector combined;
    combined.insert(combined.end(), value1.begin(), value1.end());
    combined.insert(combined.end(), value2.begin(), value2.end());
    ByteVector expectedHash = hashFunction.hash(combined);
    
    // Verify the hash matches
    BOOST_TEST(level255.begin()->second == expectedHash);
}

BOOST_FIXTURE_TEST_CASE(test_calculate_root_from_leaves, SmtRootCalcFixture) {
    BOOST_TEST_MESSAGE("Testing calculateRootFromLeaves function");
    
    // Create a map with several leaves
    std::map<uint256_t, ByteVector> leaves;
    for (uint256_t i = 0; i < 10; i++) {
        ByteVector value{static_cast<uint8_t>(i), static_cast<uint8_t>(i), static_cast<uint8_t>(i)};
        leaves[i] = value;
    }
    
    // Calculate root using the new method
    ByteVector root = smt.calculateRootFromLeaves(leaves);
    
    // Add the same leaves to the SMT
    for (const auto& [key, value] : leaves) {
        smt.setLeaf(key, value);
    }
    
    // Verify the calculated root matches the SMT's root
    BOOST_TEST(root == smt.getRootHash());
}

BOOST_FIXTURE_TEST_CASE(test_calculate_root_from_current_leaves, SmtRootCalcFixture) {
    BOOST_TEST_MESSAGE("Testing calculateRootFromCurrentLeaves function");
    
    // Add several leaves to the SMT
    for (uint256_t i = 0; i < 10; i++) {
        ByteVector value{static_cast<uint8_t>(i), static_cast<uint8_t>(i), static_cast<uint8_t>(i)};
        smt.setLeaf(i, value);
    }
    
    // Get the current root
    ByteVector originalRoot = smt.getRootHash();
    
    // Calculate root using the new method
    ByteVector calculatedRoot = smt.calculateRootFromCurrentLeaves();
    
    // Verify the calculated root matches the original root
    BOOST_TEST(calculatedRoot == originalRoot);
}

BOOST_FIXTURE_TEST_CASE(test_root_calculation_performance, SmtRootCalcFixture) {
    const size_t TEST_LEAVES_COUNT = 1000;
    BOOST_TEST_MESSAGE("Testing root calculation performance with " + std::to_string(TEST_LEAVES_COUNT) + " leaves");
    
    // Create leaves
    std::map<uint256_t, ByteVector> leaves;
    for (size_t i = 0; i < TEST_LEAVES_COUNT; i++) {
        ByteVector value{static_cast<uint8_t>(i % 256), static_cast<uint8_t>(i % 256), static_cast<uint8_t>(i % 256)};
        leaves[i] = value;
        smt.setLeaf(i, value);
    }
    
    // Get the original root
    ByteVector originalRoot = smt.getRootHash();
    
    // Calculate root using the new method
    auto start = std::chrono::steady_clock::now();
    ByteVector calculatedRoot = smt.calculateRootFromLeaves(leaves);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    BOOST_TEST_MESSAGE("Root calculation time: " + std::to_string(duration.count()) + "ms");
    
    // Verify the calculated root matches the original root
    BOOST_TEST(calculatedRoot == originalRoot);
}

BOOST_FIXTURE_TEST_CASE(test_null_hash_insertion, SmtRootCalcFixture) {
    BOOST_TEST_MESSAGE("Testing insertion of null hash into leaves");
    
    // Get the null hash
    ByteVector nullHash = smt.getNullHash();
    BOOST_TEST_MESSAGE("Null hash: " + HashFunction::hashToString(nullHash));
    
    // Create a map with a single leaf containing the null hash
    std::map<uint256_t, ByteVector> leaves;
    uint256_t key = 123;
    leaves[key] = nullHash;
    
    // Calculate root using the calculateRootFromLeaves method
    ByteVector calculatedRoot = smt.calculateRootFromLeaves(leaves);
    BOOST_TEST_MESSAGE("Calculated root hash: " + HashFunction::hashToString(calculatedRoot));
    
    // Add the null hash to the SMT using the setLeafHash method
    smt.setLeafHash(key, nullHash);
    
    // Get the root hash from the SMT
    ByteVector smtRoot = smt.getRootHash();
    BOOST_TEST_MESSAGE("SMT root hash: " + HashFunction::hashToString(smtRoot));
    
    // Verify the calculated root matches the SMT root
    BOOST_TEST(calculatedRoot == smtRoot);
}

BOOST_AUTO_TEST_SUITE_END()
