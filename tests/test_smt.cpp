#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/hash_sha256.h"
#include <string>

ByteVector stringToByteVector(const std::string& str) {
    return ByteVector(str.begin(), str.end());
}

BOOST_AUTO_TEST_SUITE(SmtTests)

struct SmtFixture {
    ByteVector defaultValue{0x00};
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{defaultValue, hashFunction};
};

BOOST_FIXTURE_TEST_CASE(test_initialization, SmtFixture) {
    // Verify that the context was created successfully
    BOOST_TEST(smt.getRootHash().size() == 32); // SHA256 produces 32-byte hashes
}

BOOST_FIXTURE_TEST_CASE(test_zero_hashes, SmtFixture) {
    BOOST_TEST(!ZERO_HASHES.empty());
    BOOST_TEST(ZERO_HASHES.size() == 257); // Should have hashes from depth 0 to 256
    
    // Check sizes
    for (const auto& hash : ZERO_HASHES) {
        BOOST_TEST(hash.size() == 32);
    }
    
    // Verify that level 0 is the hash of empty input
    ByteVector emptyHash = hashFunction.hash(ByteVector());
    BOOST_TEST(ZERO_HASHES[0] == emptyHash);
    
    // Iterate over the array using conat child 2x hashes
    for (size_t i = 1; i <= 256; i++) {
        ByteVector combined;
        combined.insert(combined.end(), ZERO_HASHES[i-1].begin(), ZERO_HASHES[i-1].end());
        combined.insert(combined.end(), ZERO_HASHES[i-1].begin(), ZERO_HASHES[i-1].end());
        ByteVector expectedHash = hashFunction.hash(combined);
        BOOST_TEST(ZERO_HASHES[i] == expectedHash);
    }
}

BOOST_FIXTURE_TEST_CASE(test_root_hash, SmtFixture) {
    BOOST_TEST(smt.getRootHash() == ZERO_HASHES[256]);
    
    // Test string representation - for printing results
    std::string rootHashString = smt.getRootHashString();
    BOOST_TEST(!rootHashString.empty());
    BOOST_TEST(rootHashString.length() == 64); // 32 Bytes Hex = 64 Characters
}

BOOST_FIXTURE_TEST_CASE(test_hash_to_string, SmtFixture) {
    // Test hash to string conversion with a known input
    ByteVector testData{0x01, 0x02, 0x03};
    ByteVector hash = hashFunction.hash(testData);
    std::string hashString = HashFunction::hashToString(hash);
    
    // Verify the string is a valid hex representation
    BOOST_TEST(hashString.length() == 64);
    BOOST_TEST(hashString.find_first_not_of("0123456789abcdef") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_string_default_value) {
    ByteVector defaultValue = stringToByteVector("empty");
    Sha256HashFunction hashFunc;
    SmtContext<Sha256HashFunction> smt(defaultValue, hashFunc);
    
    // Verify the root hash is properly initialized
    BOOST_TEST(smt.getRootHash().size() == 32);
    BOOST_TEST(smt.getRootHash() == ZERO_HASHES[256]);
    
    // Verify we can get the root hash as a string
    std::string rootHashString = smt.getRootHashString();
    BOOST_TEST(!rootHashString.empty());
    BOOST_TEST(rootHashString.length() == 64);
}

BOOST_AUTO_TEST_SUITE_END()
