cmake_minimum_required(VERSION 3.10)
project(smt_web_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.11.1
)
FetchContent_MakeAvailable(pybind11)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

add_library(smt_impl
    ../src/hash_function.cpp
    ../src/hash_sha256.cpp
    ../src/proof.cpp
    src/globals.cpp
)
set_property(TARGET smt_impl PROPERTY POSITION_INDEPENDENT_CODE ON)

# Add the Python module
pybind11_add_module(smt_bindings src/bindings.cpp)
target_link_libraries(smt_bindings PRIVATE ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} smt_impl)
