#include "node.h"

Node::Node() : hash(), left(nullptr), right(nullptr) {}
Node::Node(const ByteVector& h) : hash(h), left(nullptr), right(nullptr) {}
