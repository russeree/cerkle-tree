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
    BOOST_TEST_MESSAGE("=== test_proof_single_item ===");
    
    uint256_t key = 123;
    ByteVector value{0x01, 0x02, 0x03};
    
    BOOST_TEST_MESSAGE("Setting leaf for key=" + std::to_string(key.convert_to<unsigned long long>()));
    smt.setLeaf(key, value);
    
    BOOST_TEST_MESSAGE("Root hash after setting leaf: " + HashFunction::hashToString(smt.getRootHash()));
    
    BOOST_TEST_MESSAGE("Generating proof for key=" + std::to_string(key.convert_to<unsigned long long>()));
    MerkleProof proof = smt.generateProof(key);
    
    BOOST_TEST_MESSAGE("Proof size: " + std::to_string(proof.size()));
    BOOST_TEST_MESSAGE("Proof valid: " + std::string(proof.isValid() ? "true" : "false"));
    
    // Print the first few sibling hashes in the proof
    BOOST_TEST_MESSAGE("First few sibling hashes in proof:");
    for (size_t i = 0; i < 5 && i < proof.size(); i++) {
        BOOST_TEST_MESSAGE("  Level " + std::to_string(i) + ": " + 
                          HashFunction::hashToString(proof.getSibling(i)));
    }
    
    BOOST_TEST_MESSAGE("Validating proof with key and value");
    bool validationResult = smt.validateProof(key, value, proof);
    BOOST_TEST_MESSAGE("Validation result: " + std::string(validationResult ? "true" : "false"));
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    BOOST_TEST(validationResult);
}

BOOST_FIXTURE_TEST_CASE(test_proof_single_leaf, SmtProofFixture) {
    BOOST_TEST_MESSAGE("=== test_proof_single_leaf ===");
    
    uint256_t key = 42;
    ByteVector value{0x01, 0x02, 0x03};
    
    BOOST_TEST_MESSAGE("Setting leaf for key=" + std::to_string(key.convert_to<unsigned long long>()));
    smt.setLeaf(key, value);
    
    BOOST_TEST_MESSAGE("Root hash after setting leaf: " + HashFunction::hashToString(smt.getRootHash()));
    
    BOOST_TEST_MESSAGE("Generating proof for key=" + std::to_string(key.convert_to<unsigned long long>()));
    MerkleProof proof = smt.generateProof(key);
    
    BOOST_TEST_MESSAGE("Proof size: " + std::to_string(proof.size()));
    BOOST_TEST_MESSAGE("Proof valid: " + std::string(proof.isValid() ? "true" : "false"));
    
    // Print the first few sibling hashes in the proof
    BOOST_TEST_MESSAGE("First few sibling hashes in proof:");
    for (size_t i = 0; i < 5 && i < proof.size(); i++) {
        BOOST_TEST_MESSAGE("  Level " + std::to_string(i) + ": " + 
                          HashFunction::hashToString(proof.getSibling(i)));
    }
    
    BOOST_TEST_MESSAGE("Validating proof with key and value");
    bool validationResult = smt.validateProof(key, value, proof);
    BOOST_TEST_MESSAGE("Validation result: " + std::string(validationResult ? "true" : "false"));
    BOOST_TEST(proof.isValid());
    BOOST_TEST(validationResult);
    
    ByteVector wrongValue{0x04, 0x05, 0x06};
    BOOST_TEST_MESSAGE("Validating proof with key and wrong value (should fail)");
    bool wrongValidationResult = smt.validateProof(key, wrongValue, proof);
    BOOST_TEST_MESSAGE("Wrong validation result: " + std::string(wrongValidationResult ? "true" : "false"));
    BOOST_TEST(!wrongValidationResult);
}

BOOST_FIXTURE_TEST_CASE(test_proof_multiple_leaves, SmtProofFixture) {
    BOOST_TEST_MESSAGE("=== test_proof_multiple_leaves ===");
    
    // Add several leaves
    uint256_t key1 = 1;
    uint256_t key2 = 999;
    ByteVector value1{0x01, 0x01, 0x01};
    ByteVector value2{0x02, 0x02, 0x02};
    
    BOOST_TEST_MESSAGE("Setting leaf for key1=" + std::to_string(key1.convert_to<unsigned long long>()));
    smt.setLeaf(key1, value1);
    BOOST_TEST_MESSAGE("Setting leaf for key2=" + std::to_string(key2.convert_to<unsigned long long>()));
    smt.setLeaf(key2, value2);
    
    BOOST_TEST_MESSAGE("Root hash after setting leaves: " + HashFunction::hashToString(smt.getRootHash()));
    
    // Generate and validate proof for key1
    BOOST_TEST_MESSAGE("Generating proof for key1=" + std::to_string(key1.convert_to<unsigned long long>()));
    MerkleProof proof1 = smt.generateProof(key1);
    BOOST_TEST_MESSAGE("Proof1 size: " + std::to_string(proof1.size()));
    
    // Print the first few sibling hashes in proof1
    BOOST_TEST_MESSAGE("First few sibling hashes in proof1:");
    for (size_t i = 0; i < 5 && i < proof1.size(); i++) {
        BOOST_TEST_MESSAGE("  Level " + std::to_string(i) + ": " + 
                          HashFunction::hashToString(proof1.getSibling(i)));
    }
    
    BOOST_TEST_MESSAGE("Validating proof1 with key1 and value1");
    bool validation1 = smt.validateProof(key1, value1, proof1);
    BOOST_TEST_MESSAGE("Validation1 result: " + std::string(validation1 ? "true" : "false"));
    BOOST_TEST(validation1);
    
    // Generate and validate proof for key2
    BOOST_TEST_MESSAGE("Generating proof for key2=" + std::to_string(key2.convert_to<unsigned long long>()));
    MerkleProof proof2 = smt.generateProof(key2);
    BOOST_TEST_MESSAGE("Proof2 size: " + std::to_string(proof2.size()));
    
    // Print the first few sibling hashes in proof2
    BOOST_TEST_MESSAGE("First few sibling hashes in proof2:");
    for (size_t i = 0; i < 5 && i < proof2.size(); i++) {
        BOOST_TEST_MESSAGE("  Level " + std::to_string(i) + ": " + 
                          HashFunction::hashToString(proof2.getSibling(i)));
    }
    
    BOOST_TEST_MESSAGE("Validating proof2 with key2 and value2");
    bool validation2 = smt.validateProof(key2, value2, proof2);
    BOOST_TEST_MESSAGE("Validation2 result: " + std::string(validation2 ? "true" : "false"));
    BOOST_TEST(validation2);
    
    // Cross-validation tests
    BOOST_TEST_MESSAGE("Cross-validation tests:");
    
    BOOST_TEST_MESSAGE("Validating proof1 with key1 and value2 (should fail)");
    bool crossValidation1 = smt.validateProof(key1, value2, proof1);
    BOOST_TEST_MESSAGE("Result: " + std::string(crossValidation1 ? "true" : "false"));
    BOOST_TEST(!crossValidation1);
    
    BOOST_TEST_MESSAGE("Validating proof2 with key2 and value1 (should fail)");
    bool crossValidation2 = smt.validateProof(key2, value1, proof2);
    BOOST_TEST_MESSAGE("Result: " + std::string(crossValidation2 ? "true" : "false"));
    BOOST_TEST(!crossValidation2);
    
    BOOST_TEST_MESSAGE("Validating proof1 with key2 and value2 (should fail)");
    bool crossValidation3 = smt.validateProof(key2, value2, proof1);
    BOOST_TEST_MESSAGE("Result: " + std::string(crossValidation3 ? "true" : "false"));
    BOOST_TEST(!crossValidation3);
    
    BOOST_TEST_MESSAGE("Validating proof2 with key1 and value1 (should fail)");
    bool crossValidation4 = smt.validateProof(key1, value1, proof2);
    BOOST_TEST_MESSAGE("Result: " + std::string(crossValidation4 ? "true" : "false"));
    BOOST_TEST(!crossValidation4);
}

BOOST_FIXTURE_TEST_CASE(test_proof_null_hash_at_index_zero, SmtProofFixture) {
    BOOST_TEST_MESSAGE("=== test_proof_null_hash_at_index_zero ===");
    
    // Create an empty SMT (no leaves added)
    
    // Get the null hash
    ByteVector nullHash = smt.getNullHash();
    BOOST_TEST_MESSAGE("Null hash: " + HashFunction::hashToString(nullHash));
    
    // Get the root hash of the empty tree
    ByteVector rootHash = smt.getRootHash();
    BOOST_TEST_MESSAGE("Root hash: " + HashFunction::hashToString(rootHash));
    
    // Generate a proof for key 0
    uint256_t key = 0;
    MerkleProof proof = smt.generateProof(key);
    
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    
    // Print all the zero hashes for reference
    BOOST_TEST_MESSAGE("Zero hashes:");
    for (size_t i = 0; i <= 5; i++) { // Just print the first few for brevity
        BOOST_TEST_MESSAGE("Level " + std::to_string(i) + ": " + 
                          HashFunction::hashToString(smt.getZeroHash(i)));
    }
    
    // Print the sibling hashes in the proof
    BOOST_TEST_MESSAGE("Sibling hashes in proof:");
    for (size_t i = 0; i < 5; i++) { // Just print the first few for brevity
        BOOST_TEST_MESSAGE("Level " + std::to_string(i) + ": " + 
                          HashFunction::hashToString(proof.getSibling(i)));
    }
    
    // Manually validate the proof to see where it fails
    uint256_t currentKey = key;
    ByteVector currentHash = nullHash;
    
    BOOST_TEST_MESSAGE("Starting manual validation with key: " + std::to_string(currentKey.convert_to<unsigned long long>()));
    BOOST_TEST_MESSAGE("Starting hash: " + HashFunction::hashToString(currentHash));
    
    for (int level = 256; level > 250; level--) { // Just validate the first few levels for brevity
        bool isLeftNode = (currentKey & 1) == 0;
        BOOST_TEST_MESSAGE("Level " + std::to_string(level) + ", Key: " + 
                          std::to_string(currentKey.convert_to<unsigned long long>()) + 
                          ", Is Left: " + (isLeftNode ? "true" : "false"));
        
        ByteVector siblingHash = proof.getSibling(256-level);
        BOOST_TEST_MESSAGE("  Sibling hash: " + HashFunction::hashToString(siblingHash));
        
        ByteVector combined;
        if (isLeftNode) {
            combined.insert(combined.end(), currentHash.begin(), currentHash.end());
            combined.insert(combined.end(), siblingHash.begin(), siblingHash.end());
            BOOST_TEST_MESSAGE("  Combined (left+right): " + HashFunction::hashToString(combined));
        } else {
            combined.insert(combined.end(), siblingHash.begin(), siblingHash.end());
            combined.insert(combined.end(), currentHash.begin(), currentHash.end());
            BOOST_TEST_MESSAGE("  Combined (right+left): " + HashFunction::hashToString(combined));
        }
        
        currentHash = hashFunction.hash(combined);
        BOOST_TEST_MESSAGE("  New hash: " + HashFunction::hashToString(currentHash));
        
        currentKey >>= 1;
        BOOST_TEST_MESSAGE("  New key: " + std::to_string(currentKey.convert_to<unsigned long long>()));
    }
    
    BOOST_TEST_MESSAGE("Final hash: " + HashFunction::hashToString(currentHash));
    BOOST_TEST_MESSAGE("Root hash: " + HashFunction::hashToString(rootHash));
    
    // Verify that the proof validates correctly with the null hash
    bool validationResult = smt.validateProof(key, nullHash, proof);
    BOOST_TEST_MESSAGE("Validation result: " + std::string(validationResult ? "true" : "false"));
    BOOST_TEST(validationResult);
}

BOOST_AUTO_TEST_SUITE_END()
