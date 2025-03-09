#include <boost/test/unit_test.hpp>
#include "../src/smt.h"
#include "../src/hash_sha256.h"
#include "test_utils.h"

BOOST_AUTO_TEST_SUITE(SmtNodeTests)

struct SmtNodeFixture {
    Sha256HashFunction hashFunction;
    SmtContext<Sha256HashFunction> smt{hashFunction};
    
    // Helper function to create test values
    ByteVector createTestValue(uint8_t val) {
        return ByteVector{val, val, val, val};
    }
};

BOOST_FIXTURE_TEST_CASE(test_leaf_operations, SmtNodeFixture) {
    uint256_t key1(0);  // Left node (ends in 0)
    uint256_t key2(1);  // Right node (ends in 1)
    ByteVector value1 = createTestValue(0x11);
    ByteVector value2 = createTestValue(0x22);
    
    // Test setting and getting leaves
    smt.setLeaf(key1, value1);
    smt.setLeaf(key2, value2);
    
    BOOST_TEST(smt.hasLeaf(key1));
    BOOST_TEST(smt.hasLeaf(key2));
    BOOST_TEST(!smt.hasLeaf(uint256_t(2)));  // Non-existent key
    
    BOOST_TEST(smt.getLeaf(key1) == hashFunction.hash(value1));
    BOOST_TEST(smt.getLeaf(key2) == hashFunction.hash(value2));
    BOOST_TEST(smt.getLeaf(uint256_t(2)) == hashFunction.hash(ByteVector()));
}

BOOST_FIXTURE_TEST_CASE(test_node_position, SmtNodeFixture) {
    // Small Node Keys
    uint256_t leftNode1(0);     // ...000
    uint256_t rightNode1(1);    // ...001
    uint256_t leftNode2(2);     // ...010
    uint256_t rightNode2(255);  // ...1111111
    // Larger Node Keys
    uint256_t leftNode3("1234567890"); 
    uint256_t rightNode3 = leftNode3 + 1;
    
    // Test isLeft function
    BOOST_TEST(smt.isLeft(leftNode1));
    BOOST_TEST(!smt.isLeft(rightNode1));
    BOOST_TEST(smt.isLeft(leftNode2));
    BOOST_TEST(!smt.isLeft(rightNode2));
    BOOST_TEST(smt.isLeft(leftNode3));
    BOOST_TEST(!smt.isLeft(rightNode3));
}

BOOST_FIXTURE_TEST_CASE(test_paired_nodes, SmtNodeFixture) {
    // Test paired node relationships
    uint256_t leftNode(0);    // ...000
    uint256_t rightNode(1);   // ...001
    uint256_t leftNode2(2);   // ...010
    uint256_t rightNode2(3);  // ...011
    
    // Test getting paired nodes
    BOOST_TEST(smt.getPairedNode(leftNode) == rightNode);
    BOOST_TEST(smt.getPairedNode(rightNode) == leftNode);
    BOOST_TEST(smt.getPairedNode(leftNode2) == rightNode2);
    BOOST_TEST(smt.getPairedNode(rightNode2) == leftNode2);
    
    // Test with larger numbers
    uint256_t largeLeft("1234567890");  // Assuming even number
    uint256_t largeRight = largeLeft + 1;
    BOOST_TEST(smt.getPairedNode(largeLeft) == largeRight);
    BOOST_TEST(smt.getPairedNode(largeRight) == largeLeft);
}

BOOST_AUTO_TEST_SUITE_END()
