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

unset(BUILD_TESTING)
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

set(Protobuf_USE_STATIC_LIBS ON)
option(protobuf_INSTALL OFF)
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG         v21.9
)

FetchContent_MakeAvailable(spdlog msft_proxy Boost protobuf)

include (GenerateProto)

generate_proto(
    FILE proto/mrpc.proto
    GENERATE_PATH ${CMAKE_CURRENT_BINARY_DIR}/proto
    OUTPUT_FILE MRPC_RPC_PROTO_SRCS
    IMPORT_PATH proto/ ${MRPC_PROTO_IMPORT_PATH}
)

add_library(mrpc_proto INTERFACE)
target_include_directories(mrpc_proto INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/proto)
add_library(mrpc::proto ALIAS mrpc_proto)

set(MRPC_SRCS
    "include/mrpc/detail/error_code.hpp"
    "src/mrpc/detail/error_code.cpp"
    "include/mrpc/await_error.hpp"
    "include/mrpc/error_code.hpp"
    "include/mrpc/method.hpp")

add_library(mrpc STATIC ${MRPC_SRCS} ${MRPC_RPC_PROTO_SRCS})
target_link_libraries(mrpc PUBLIC protobuf::libprotobuf Boost::asio Boost::system msft_proxy)
target_include_directories(mrpc PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/proto include/)
add_library(mrpc::mrpc ALIAS mrpc)

if (MRPC_BUILD_TESTS)
    add_executable(test_proto_gen "test/test_proto_gen.cpp")
    target_link_libraries(test_proto_gen mrpc_proto_gen)
endif()