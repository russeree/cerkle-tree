#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/proof.h"
#include "test_utils.h"

BOOST_AUTO_TEST_SUITE(SmtProofTests)

struct SmtProofFixture {
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{hashFunction};
};

BOOST_FIXTURE_TEST_CASE(test_proof_single_item, SmtProofFixture) {
    uint256_t key = 123;
    ByteVector value{0x01, 0x02, 0x03};
    smt.setLeaf(key, value);
    MerkleProof proof = smt.generateProof(key);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    BOOST_TEST(smt.validateProof(key, value, proof));
}

BOOST_FIXTURE_TEST_CASE(test_proof_single_leaf, SmtProofFixture) {
    uint256_t key = 42;
    ByteVector value{0x01, 0x02, 0x03};
    
    smt.setLeaf(key, value);
    MerkleProof proof = smt.generateProof(key);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(smt.validateProof(key, value, proof));
    
    ByteVector wrongValue{0x04, 0x05, 0x06};
    BOOST_TEST(!smt.validateProof(key, wrongValue, proof));
}

BOOST_FIXTURE_TEST_CASE(test_proof_multiple_leaves, SmtProofFixture) {
    // Add several leaves
    uint256_t key1 = 1;
    uint256_t key2 = 999;
    ByteVector value1{0x01, 0x01, 0x01};
    ByteVector value2{0x02, 0x02, 0x02};
    
    smt.setLeaf(key1, value1);
    smt.setLeaf(key2, value2);
    
    MerkleProof proof1 = smt.generateProof(key1);
    BOOST_TEST(smt.validateProof(key1, value1, proof1));
    
    MerkleProof proof2 = smt.generateProof(key2);
    BOOST_TEST(smt.validateProof(key2, value2, proof2));
    
    BOOST_TEST(!smt.validateProof(key1, value2, proof1));
    BOOST_TEST(!smt.validateProof(key2, value1, proof2));
    BOOST_TEST(!smt.validateProof(key2, value2, proof1));
    BOOST_TEST(!smt.validateProof(key1, value1, proof2));
}

BOOST_FIXTURE_TEST_CASE(test_non_inclusion_proof_empty_tree, SmtProofFixture) {
    uint256_t key = 123;
    MerkleProof proof = smt.generateProof(key);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    BOOST_TEST(smt.validateNonInclusionProof(key, proof));
}

BOOST_FIXTURE_TEST_CASE(test_non_inclusion_proof_with_other_leaves, SmtProofFixture) {
    // Add some leaves to the tree
    uint256_t existingKey1 = 42;
    uint256_t existingKey2 = 999;
    ByteVector value1{0x01, 0x02, 0x03};
    ByteVector value2{0x04, 0x05, 0x06};
    
    smt.setLeaf(existingKey1, value1);
    smt.setLeaf(existingKey2, value2);
    
    // Test non-inclusion proof for a non-existent key
    uint256_t nonExistentKey = 123;
    MerkleProof proof = smt.generateProof(nonExistentKey);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(smt.validateNonInclusionProof(nonExistentKey, proof));
    
    // Non-inclusion proof should fail for existing keys
    MerkleProof proofExisting = smt.generateProof(existingKey1);
    BOOST_TEST(!smt.validateNonInclusionProof(existingKey1, proofExisting));
}

BOOST_FIXTURE_TEST_CASE(test_non_inclusion_after_removal, SmtProofFixture) {
    // Add and then remove a leaf
    uint256_t key = 42;
    ByteVector value{0x01, 0x02, 0x03};
    
    smt.setLeaf(key, value);
    smt.removeLeaf(key);
    
    // Should now be able to prove non-inclusion
    MerkleProof proof = smt.generateProof(key);
    BOOST_TEST(proof.isValid());
    BOOST_TEST(smt.validateNonInclusionProof(key, proof));
}

BOOST_AUTO_TEST_SUITE_END()
