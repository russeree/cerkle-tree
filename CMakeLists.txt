cmake_minimum_required(VERSION 3.14)
project(cerkle-tree VERSION 0.0.1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

add_library(${PROJECT_NAME}
    src/hash_function.cpp
    src/hash_sha256.cpp
    src/proof.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::Crypto)

enable_testing()

add_executable(${PROJECT_NAME}_test
    tests/test_main.cpp
    tests/test_smt.cpp
    tests/test_smt_initialization.cpp
    tests/test_smt_hashing.cpp
    tests/test_smt_nodes.cpp
    tests/test_smt_proofs.cpp
    tests/test_smt_non_inclusion_proofs.cpp
)

target_link_libraries(${PROJECT_NAME}_test 
    PRIVATE 
    ${PROJECT_NAME}
    Boost::unit_test_framework
)

add_test(
    NAME ${PROJECT_NAME}_test 
    COMMAND ${PROJECT_NAME}_test 
        --log_level=all 
        --report_level=detailed 
        --color_output=yes 
        --show_progress=yes
)
