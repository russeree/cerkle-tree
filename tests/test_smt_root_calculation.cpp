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

BOOST_FIXTURE_TEST_CASE(test_calculate_root_from_current_leaves, SmtRootCalcFixture) {
    std::cout << "Starting test_calculate_root_from_current_leaves" << std::endl;
    
    // Add several leaves to the SMT
    std::cout << "Adding 10 leaves to the SMT..." << std::endl;
    for (uint256_t i = 0; i < 10; i++) {
        ByteVector value{static_cast<uint8_t>(i), static_cast<uint8_t>(i), static_cast<uint8_t>(i)};
        std::cout << "  Setting leaf at index " << i << " with value [" 
                  << static_cast<int>(value[0]) << "," 
                  << static_cast<int>(value[1]) << "," 
                  << static_cast<int>(value[2]) << "]" << std::endl;
        smt.setLeaf(i, value);
    }
    
    // Get the current root
    std::cout << "Getting the current root hash..." << std::endl;
    ByteVector originalRoot = smt.getRootHash();
    std::cout << "Original root hash: " << bytesToHexString(originalRoot) << std::endl;
    
    // Calculate root using the new method
    std::cout << "Calculating root using calculateRootFromCurrentLeaves()..." << std::endl;
    ByteVector calculatedRoot = smt.calculateRootFromCurrentLeaves();
    std::cout << "Calculated root hash: " << bytesToHexString(calculatedRoot) << std::endl;
    
    // Verify the calculated root matches the original root
    std::cout << "Verifying calculated root matches original root..." << std::endl;
    bool rootsMatch = (calculatedRoot == originalRoot);
    std::cout << "Roots match: " << (rootsMatch ? "YES" : "NO") << std::endl;
    BOOST_TEST(calculatedRoot == originalRoot);
    
    std::cout << "test_calculate_root_from_current_leaves completed" << std::endl;
}

BOOST_FIXTURE_TEST_CASE(test_null_hash_insertion, SmtRootCalcFixture) {
    std::cout << "Starting test_null_hash_insertion" << std::endl;
    
    // Get the null hash
    std::cout << "Getting the null hash..." << std::endl;
    ByteVector nullHash = smt.getNullHash();
    std::cout << "Null hash: " << bytesToHexString(nullHash) << std::endl;
    
    // Create a map with a single leaf containing the null hash
    std::cout << "Creating a map with a single leaf containing the null hash..." << std::endl;
    std::map<uint256_t, ByteVector> leaves;
    uint256_t key = 123;
    leaves[key] = nullHash;
    std::cout << "Added null hash at key: " << key << std::endl;
    
    // Calculate root using the calculateRootFromLeaves method
    std::cout << "Calculating root using calculateRootFromLeaves()..." << std::endl;
    ByteVector calculatedRoot = smt.calculateRootFromLeaves(leaves);
    std::cout << "Calculated root hash: " << bytesToHexString(calculatedRoot) << std::endl;
    
    // Add the null hash to the SMT using the setLeafHash method
    std::cout << "Adding the null hash to the SMT using setLeafHash()..." << std::endl;
    smt.setLeafHash(key, nullHash);
    
    // Get the root hash from the SMT
    std::cout << "Getting the SMT root hash..." << std::endl;
    ByteVector smtRoot = smt.getRootHash();
    std::cout << "SMT root hash: " << bytesToHexString(smtRoot) << std::endl;
    
    // Verify the calculated root matches the SMT root
    std::cout << "Verifying calculated root matches SMT root..." << std::endl;
    bool rootsMatch = (calculatedRoot == smtRoot);
    std::cout << "Roots match: " << (rootsMatch ? "YES" : "NO") << std::endl;
    BOOST_TEST(calculatedRoot == smtRoot);
    
    std::cout << "test_null_hash_insertion completed" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
