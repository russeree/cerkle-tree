#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/proof.h"
#include "test_utils.h"

BOOST_AUTO_TEST_SUITE(SmtNonInclusionProofTests)

struct SmtNonInclusionProofFixture {
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{hashFunction};
};

BOOST_FIXTURE_TEST_CASE(test_empty_tree_non_inclusion_proof, SmtNonInclusionProofFixture) {
    // Test that we can generate a proof for any key in an empty tree
    // and verify it as a valid non-inclusion proof
    uint256_t key = 123;
    
    // Generate proof for non-existent key in empty tree
    MerkleProof proof = smt.generateProof(key);
    
    // Verify the proof is valid
    BOOST_TEST(proof.isValid());
    BOOST_TEST(proof.size() == 256);
    BOOST_TEST(smt.validateNonInclusionProof(key, proof));
    BOOST_TEST(smt.getRootHash() == SmtContext<>::getZeroHash(256));
}

BOOST_AUTO_TEST_SUITE_END()
