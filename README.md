# Cerkle Tree

A fast and efficient C++ implementation of a Sparse Merkle Tree library using OpenSSL. 

# Notes
❌ Do not use in production. NOT AUDITED

## Notes and Thoughts
[Notes Gist](https://gist.github.com/russeree/ad594779d1b3dc978495beab712311dd) 

## Overview

Cerkle Tree provides a templated implementation of Sparse Merkle Trees with configurable hash functions. _The default implementation uses SHA-256 via OpenSSL._ 

## Documentation

- [Path Finding Algorithm](docs/path_algorithm.md) - Algorithm for determining the intersection of leaves in a Sparse Merkle Tree

## SMT Playground - Visualizer
![image](https://github.com/user-attachments/assets/922f6124-475a-46d0-b272-8838d070584f)

## Dependencies

- CMake (>= 3.14)
- OpenSSL
- Boost (for testing)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

This will generate:
- `libcerkle-tree.a`: The static library
- `cerkle-tree_test`: The test executable

## Testing

Tests are implemented using the Boost Test Framework. To run the tests:

```bash
cd build
ctest --output-on-failure
```

## Usage

```cpp
#include "smt.h"
#include "hash_sha256.h"

// Create a new SMT context (uses empty ByteVector as default value)
Sha256HashFunction hashFunc;
SmtContext<Sha256HashFunction> smt(hashFunc);

// Get the root hash
ByteVector rootHash = smt.getRootHash();
std::string rootHashString = smt.getRootHashString();
```

## License

This project is open source and available under the MIT License.
