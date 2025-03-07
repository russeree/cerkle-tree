#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/hash_sha256.h"
#include "test_utils.h"

BOOST_AUTO_TEST_SUITE(SmtHashingTests)

struct SmtHashFixture {
    ByteVector defaultValue{0x00};
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{defaultValue, hashFunction};
};

BOOST_FIXTURE_TEST_CASE(test_zero_hashes, SmtHashFixture) {
    BOOST_TEST_MESSAGE("Testing zero hashes generation and validation");
    std::vector<std::string> errors;

    // Basic Checks
    if (ZERO_HASHES.empty()) {
        errors.push_back("ZERO_HASHES is empty");
    }
    if (ZERO_HASHES.size() != 257) {
        errors.push_back("ZERO_HASHES size is " + std::to_string(ZERO_HASHES.size()) + ", expected 257");
    }
    
    // Check the sizes of our hashes (32 Bytes | 256 Bits)
    for (size_t i = 0; i < ZERO_HASHES.size(); i++) {
        if (ZERO_HASHES[i].size() != 32) {
            errors.push_back("Hash at level " + std::to_string(i) + " has incorrect size: " + 
                           std::to_string(ZERO_HASHES[i].size()) + ", expected 32");
        }
    }
    
    // Verify Leaf hash
    ByteVector emptyHash = hashFunction.hash(ByteVector());
    if (ZERO_HASHES[0] != emptyHash) {
        errors.push_back("Level 0 hash mismatch");
    }
    
    // Climb down the tree to the root.
    for (size_t i = 1; i <= 256 && i < ZERO_HASHES.size(); i++) {
        ByteVector combined;
        combined.insert(combined.end(), ZERO_HASHES[i-1].begin(), ZERO_HASHES[i-1].end());
        combined.insert(combined.end(), ZERO_HASHES[i-1].begin(), ZERO_HASHES[i-1].end());
        ByteVector expectedHash = hashFunction.hash(combined);
        if (ZERO_HASHES[i] != expectedHash) {
            errors.push_back("Hash mismatch at level " + std::to_string(i));
        }
    }

    if (errors.empty()) {
        BOOST_TEST_MESSAGE("All zero hashes verified successfully");
        BOOST_TEST(true);
    } else {
        BOOST_TEST_MESSAGE("Zero hashes validation failed:");
        for (const auto& error : errors) {
            BOOST_TEST_MESSAGE("  - " + error);
        }
        BOOST_TEST(false);
    }
}

BOOST_FIXTURE_TEST_CASE(test_hash_to_string, SmtHashFixture) {
    BOOST_TEST_MESSAGE("Testing hash to string conversion");
    std::vector<std::string> errors;
    
    // Hash to string with a known input
    ByteVector testData{0x01, 0x02, 0x03};
    ByteVector hash = hashFunction.hash(testData);
    std::string hashString = HashFunction::hashToString(hash);
    
    // Verify the string is valid hex
    if (hashString.length() != 64) {
        errors.push_back("Hash string length is " + std::to_string(hashString.length()) + ", expected 64");
    }
    if (hashString.find_first_not_of("0123456789abcdef") != std::string::npos) {
        errors.push_back("Hash string contains invalid characters");
    }

    if (errors.empty()) {
        BOOST_TEST_MESSAGE("Hash to string conversion verified successfully");
        BOOST_TEST(true);
    } else {
        BOOST_TEST_MESSAGE("Hash to string validation failed:");
        for (const auto& error : errors) {
            BOOST_TEST_MESSAGE("  - " + error);
        }
        BOOST_TEST(false);
    }
}

BOOST_AUTO_TEST_SUITE_END()
