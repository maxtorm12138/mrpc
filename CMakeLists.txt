cmake_minimum_required(VERSION 3.14)
project(mrpc LANGUAGES CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.11.0
)

FetchContent_Declare(
    msft_proxy
    GIT_REPOSITORY https://github.com/microsoft/proxy.git
    GIT_TAG        1.1.1
)

FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG        boost-1.81.0.beta1
    GIT_SHALLOW TRUE
)


FetchContent_MakeAvailable(spdlog msft_proxy Boost)

set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)
include (GenerateProto)

generate_proto(
    FILE proto/mrpc.proto
    GENERATE_PATH ${CMAKE_CURRENT_BINARY_DIR}/proto
    OUTPUT_FILE MRPC_RPC_PROTO_FILES
    IMPORT_PATH proto/ ${MRPC_PROTO_IMPORT_PATH}
)

set(MRPC_PROTO_IMPORT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto PARENT_SCOPE)

add_library(mrpc_proto_gen ${MRPC_RPC_PROTO_FILES})
target_link_libraries(mrpc_proto_gen PUBLIC protobuf::libprotobuf)
target_include_directories(mrpc_proto_gen PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/proto)

set(MRPC_SRCS
    "include/await_error.hpp")

add_library(mrpc STATIC ${MRPC_SRCS})
target_link_libraries(mrpc PUBLIC mrpc_proto_gen Boost::asio Boost::system)