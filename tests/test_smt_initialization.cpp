#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/hash_sha256.h"
#include "test_utils.h"

BOOST_AUTO_TEST_SUITE(SmtInitializationTests)

struct SmtInitFixture {
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{hashFunction};
};

BOOST_FIXTURE_TEST_CASE(test_basic_initialization, SmtInitFixture) {
    BOOST_TEST_MESSAGE("Testing basic SMT initialization");
    BOOST_TEST(smt.getRootHash().size() == 32); // SHA256 (32-byte hashes)
}

BOOST_FIXTURE_TEST_CASE(test_root_hash, SmtInitFixture) {
    BOOST_TEST_MESSAGE("Testing root hash initialization");
    BOOST_TEST(smt.getRootHash() == ZERO_HASHES[256]);
    
    // Printing Tests
    std::string rootHashString = smt.getRootHashString();
    BOOST_TEST(!rootHashString.empty());
    BOOST_TEST(rootHashString.length() == 64); // 32 Bytes Hex = 64 Characters
}

BOOST_AUTO_TEST_CASE(test_default_initialization) {
    BOOST_TEST_MESSAGE("Testing initialization with default empty value");
    Sha256HashFunction hashFunc;
    SmtContext<Sha256HashFunction> smt(hashFunc);
    
    BOOST_TEST(smt.getRootHash().size() == 32);
    BOOST_TEST(smt.getRootHash() == ZERO_HASHES[256]);
    
    std::string rootHashString = smt.getRootHashString();
    BOOST_TEST(!rootHashString.empty());
    BOOST_TEST(rootHashString.length() == 64);
}

BOOST_AUTO_TEST_SUITE_END()
