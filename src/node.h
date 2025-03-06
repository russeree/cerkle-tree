#ifndef NODE_H
#define NODE_H

#include "hash_function.h"
#include <memory>

/**
 * @brief Node structure for the Sparse Merkle Tree : Still need to take a look and ifgure out the best way to handle the KV representation at sizes of 256 bits 
 */
struct Node {
    ByteVector hash;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
    
    Node();
    explicit Node(const ByteVector& h);
};

#endif // NODE_H
