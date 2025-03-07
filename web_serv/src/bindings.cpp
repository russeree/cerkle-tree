#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "src/smt.h"
#include "src/proof.h"
#include <string>
#include <vector>

namespace py = pybind11;

// Convert hex string to ByteVector
ByteVector hexToBytes(const std::string& hex) {
    ByteVector bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

// Convert ByteVector to hex string
std::string bytesToHex(const ByteVector& bytes) {
    std::string hex;
    for (unsigned char byte : bytes) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", byte);
        hex += buf;
    }
    return hex;
}

PYBIND11_MODULE(smt_bindings, m) {
    m.doc() = "Python bindings for Sparse Merkle Tree implementation";

    py::class_<MerkleProof>(m, "MerkleProof")
        .def(py::init<>())
        .def("add_sibling", [](MerkleProof& self, const std::string& sibling) {
            self.addSibling(hexToBytes(sibling));
        })
        .def("get_sibling", [](const MerkleProof& self, size_t level) {
            return bytesToHex(self.getSibling(level));
        })
        .def("is_valid", &MerkleProof::isValid)
        .def("size", &MerkleProof::size);

    py::class_<SmtContext<>>(m, "SMT")
        .def(py::init([]() {
            return new SmtContext<>();
        }))
        .def("get_root_hash", [](const SmtContext<>& self) {
            return bytesToHex(self.getRootHash());
        })
        .def("set_leaf", [](SmtContext<>& self, const std::string& key, const std::string& value) {
            uint256_t keyInt(key);
            self.setLeaf(keyInt, hexToBytes(value));
        })
        .def("get_leaf", [](const SmtContext<>& self, const std::string& key) {
            uint256_t keyInt(key);
            return bytesToHex(self.getLeaf(keyInt));
        })
        .def("remove_leaf", [](SmtContext<>& self, const std::string& key) {
            uint256_t keyInt(key);
            self.removeLeaf(keyInt);
        })
        .def("has_leaf", [](const SmtContext<>& self, const std::string& key) {
            uint256_t keyInt(key);
            return self.hasLeaf(keyInt);
        })
        .def("generate_proof", [](const SmtContext<>& self, const std::string& key) {
            uint256_t keyInt(key);
            return self.generateProof(keyInt);
        })
        .def("validate_proof", [](const SmtContext<>& self, const std::string& key, 
                                 const std::string& value, const MerkleProof& proof) {
            uint256_t keyInt(key);
            return self.validateProof(keyInt, hexToBytes(value), proof);
        });
}
