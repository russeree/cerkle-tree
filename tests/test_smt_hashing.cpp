#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/hash_sha256.h"
#include "test_utils.h"
#include <chrono>
#include <random>

BOOST_AUTO_TEST_SUITE(SmtHashingTests)

struct SmtHashFixture {
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{hashFunction};
};

BOOST_FIXTURE_TEST_CASE(test_zero_hashes, SmtHashFixture) {
    std::vector<std::string> errors;

    // Check hash sizes at each level (32 Bytes | 256 Bits)
    for (size_t i = 0; i <= 256; i++) {
        const ByteVector& hash = SmtContext<Sha256HashFunction>::getZeroHash(i);
        if (hash.size() != 32) {
            errors.push_back("Hash at level " + std::to_string(i) + " has incorrect size: " + 
                           std::to_string(hash.size()) + ", expected 32");
        }
    }
    
    // Verify Leaf hash
    ByteVector emptyHash = hashFunction.hash(ByteVector());
    if (SmtContext<Sha256HashFunction>::getZeroHash(0) != emptyHash) {
        errors.push_back("Level 0 hash mismatch");
    }
    
    // Verify hash chain up to root
    for (size_t i = 1; i <= 256; i++) {
        ByteVector combined;
        const ByteVector& prevHash = SmtContext<Sha256HashFunction>::getZeroHash(i-1);
        combined.insert(combined.end(), prevHash.begin(), prevHash.end());
        combined.insert(combined.end(), prevHash.begin(), prevHash.end());
        ByteVector expectedHash = hashFunction.hash(combined);
        if (SmtContext<Sha256HashFunction>::getZeroHash(i) != expectedHash) {
            errors.push_back("Hash mismatch at level " + std::to_string(i));
        }
    }

    BOOST_TEST(errors.empty());
}

BOOST_FIXTURE_TEST_CASE(test_hash_to_string, SmtHashFixture) {
    std::vector<std::string> errors;
    
    ByteVector testData{0x01, 0x02, 0x03};
    ByteVector hash = hashFunction.hash(testData);
    std::string hashString = HashFunction::hashToString(hash);
    
    if (hashString.length() != 64) {
        errors.push_back("Hash string length is " + std::to_string(hashString.length()) + ", expected 64");
    }
    if (hashString.find_first_not_of("0123456789abcdef") != std::string::npos) {
        errors.push_back("Hash string contains invalid characters");
    }

    BOOST_TEST(errors.empty());
}

struct SmtTestHelper {
    static ByteVector makeValue(uint8_t v) {
        return ByteVector{v, v, v};
    }
};

BOOST_FIXTURE_TEST_CASE(test_merkle_root_updates, SmtHashFixture) {
    // Set a single leaf and record its root
    uint256_t pos0 = 0;
    smt.setLeaf(pos0, SmtTestHelper::makeValue(0x01));
    std::string singleLeafRoot = smt.getRootHashString();
    
    // Add a second leaf and verify root changes
    uint256_t pos13371337 = 13371337;
    smt.setLeaf(pos13371337, SmtTestHelper::makeValue(0x02));
    std::string twoLeavesRoot = smt.getRootHashString();
    BOOST_TEST(singleLeafRoot != twoLeavesRoot);
    
    // Remove second leaf and verify we get original root again
    smt.removeLeaf(pos13371337);
    std::string finalRoot = smt.getRootHashString();
    BOOST_TEST(finalRoot == singleLeafRoot);
}

BOOST_FIXTURE_TEST_CASE(test_merkle_root_batch_update, SmtHashFixture) {
    const size_t TEST_LEAVES_COUNT = 100000;
    
    std::vector<std::pair<uint256_t, ByteVector>> updates;
    updates.reserve(TEST_LEAVES_COUNT);
    for (size_t i = 0; i < TEST_LEAVES_COUNT; i++) {
        updates.emplace_back(i, SmtTestHelper::makeValue(i % 256));
    }
    
    smt.setBatchLeavesHash(updates);
    
    // Verify leaves at start, middle, and end
    BOOST_TEST(smt.getLeaf(0) == SmtTestHelper::makeValue(0));
    BOOST_TEST(smt.getLeaf(TEST_LEAVES_COUNT/2) == SmtTestHelper::makeValue((TEST_LEAVES_COUNT/2) % 256));
    BOOST_TEST(smt.getLeaf(TEST_LEAVES_COUNT-1) == SmtTestHelper::makeValue((TEST_LEAVES_COUNT-1) % 256));
    
    // Remove all leaves and verify we get back to initial state
    std::vector<uint256_t> removals;
    removals.reserve(TEST_LEAVES_COUNT);
    for (size_t i = 0; i < TEST_LEAVES_COUNT; i++) {
        removals.push_back(i);
    }
    smt.removeBatchLeaves(removals);
    
    // Verify root matches initial state
    BOOST_TEST(smt.getRootHashString() == SmtContext<Sha256HashFunction>().getRootHashString());
}

BOOST_AUTO_TEST_SUITE_END()
