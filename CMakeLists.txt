cmake_minimum_required(VERSION 3.14)
project(cerkle-tree VERSION 0.0.1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

# Create the library
add_library(${PROJECT_NAME}
    src/hash_function.cpp
    src/hash_sha256.cpp
    src/smt.cpp
)

# Include directories for the library
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link OpenSSL to the library
target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::Crypto)

# Enable testing
enable_testing()

# Create test executable
add_executable(${PROJECT_NAME}_test
    tests/test_main.cpp
    tests/test_smt.cpp
)

# Link the test executable with our library and Boost
target_link_libraries(${PROJECT_NAME}_test 
    PRIVATE 
    ${PROJECT_NAME}
    Boost::unit_test_framework
)

# Add the test
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
