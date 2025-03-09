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
    bool validationResult = smt.validateProof(key, value, proof);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    BOOST_TEST(validationResult);
}

BOOST_FIXTURE_TEST_CASE(test_proof_single_leaf, SmtProofFixture) {
    uint256_t key = 42;
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
    uint256_t key1 = 133713371337;
    uint256_t key2 = 999;
    ByteVector value1{0x01, 0x01, 0x01};
    ByteVector value2{0x02, 0x02, 0x02};
    
    // Set the first leaf and test a proof
    smt.setLeaf(key1, value1);
    MerkleProof proof1 = smt.generateProof(key1);
    bool validation1 = smt.validateProof(key1, value1, proof1);
    BOOST_TEST(validation1);

    // Set the second leaf and check the proof again
    smt.setLeaf(key2, value2);
    proof1 = smt.generateProof(key1);
    bool validation1_retest = smt.validateProof(key1, value1, proof1);
    BOOST_TEST(validation1_retest);
    
    // Generate and validate proof for key2
    MerkleProof proof2 = smt.generateProof(key2);
    bool validation2 = smt.validateProof(key2, value2, proof2);
    BOOST_TEST(validation2);
    
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
    ByteVector nullHash = smt.getNullHash();
    ByteVector rootHash = smt.getRootHash();
    
    // Generate a proof for key 0
    uint256_t key = 0;
    MerkleProof proof = smt.generateProof(key);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    
    // Verify that the proof validates correctly with the null hash
    bool validationResult = smt.validateProof(key, nullHash, proof);
    BOOST_TEST(validationResult);
}

BOOST_FIXTURE_TEST_CASE(test_proof_batch_leaves, SmtProofFixture) {
    uint256_t key1 = 0;
    uint256_t key2 = 2;
    ByteVector value1{0x01, 0x01, 0x01};
    ByteVector value2{0x02, 0x02, 0x02};
    
    // Set both leaves using batch update
    std::vector<std::pair<uint256_t, ByteVector>> updates = {
        {key1, value1},
        {key2, value2}
    };
    smt.setBatchLeaves(updates);
    
    // Generate and validate proof for key1
    MerkleProof proof1 = smt.generateProof(key1);
    bool validation1 = smt.validateProof(key1, value1, proof1);
    BOOST_TEST(validation1);
    
    // Generate and validate proof for key2
    MerkleProof proof2 = smt.generateProof(key2);
    bool validation2 = smt.validateProof(key2, value2, proof2);
    BOOST_TEST(validation2);
    
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

BOOST_AUTO_TEST_SUITE_END()
