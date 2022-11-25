cmake_minimum_required(VERSION 3.14)
project(mrpc LANGUAGES CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

include (CPM)

CPMAddPackage(
    NAME msft_proxy
    VERSION 1.1.1
    GITHUB_REPOSITORY microsoft/proxy
    GIT_TAG "1.1.1"
    OPTIONS "BUILD_TESTING OFF"
)

CPMAddPackage(
  NAME spdlog
  VERSION 1.11.0
  GITHUB_REPOSITORY gabime/spdlog
  GIT_TAG "v1.11.0"
)

CPMAddPackage(
    NAME Boost
    VERSION 1.81.0
    GITHUB_REPOSITORY boostorg/boost
    GIT_TAG "boost-1.81.0.beta1"
)

CPMAddPackage(
    NAME protobuf
    VERSION 21.9
    GITHUB_REPOSITORY protocolbuffers/protobuf
    GIT_TAG "v21.9"
    OPTIONS "protobuf_INSTALL OFF" "Protobuf_USE_STATIC_LIBS ON"
)


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
    "include/mrpc/error_code.hpp"
    "src/mrpc/error_code.cpp"
    "include/mrpc/dynamic_buffer_adaptor.hpp"
    "src/mrpc/dynamic_buffer_adaptor.cpp"
    "include/mrpc/await_error.hpp"
    "src/mrpc/await_error.cpp"
    "include/mrpc/packet_handler.hpp"
    "include/mrpc/method.hpp"
    "include/mrpc/mrpc.hpp")

add_library(mrpc STATIC ${MRPC_SRCS} ${MRPC_RPC_PROTO_SRCS})
target_link_libraries(mrpc PUBLIC protobuf::libprotobuf msft_proxy Boost::system Boost::asio PRIVATE spdlog::spdlog msft_proxy)
target_include_directories(mrpc PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/proto include/)
target_compile_definitions(mrpc PUBLIC $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0A00> BOOST_ASIO_NO_DYNAMIC_BUFFER_V1 BOOST_ASIO_NO_DEPRECATED)
add_library(mrpc::mrpc ALIAS mrpc)

if (MRPC_BUILD_TESTS)

    macro(mrpc_add_test)
        cmake_parse_arguments(MRPC_TEST "" "TARGET" "FILES;LIBRARIES" ${ARGN})
        add_executable(${MRPC_TEST_TARGET} ${MRPC_TEST_FILES})
        target_link_libraries(${MRPC_TEST_TARGET} PUBLIC ${MRPC_TEST_LIBRARIES})
        target_compile_options(${MRPC_TEST_TARGET} PUBLIC $<$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","Clang">:-fno-access-control>)
        target_compile_definitions(${MRPC_TEST_TARGET} PUBLIC $<$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","Clang">:MRPC_ACCESS_PRIVATE_MEMBER>)
    endmacro()

    CPMAddPackage(
        NAME googletest
        VERSION 1.12.1
        GITHUB_REPOSITORY  google/googletest 
        GIT_TAG "release-1.12.1"
        OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt ON" "BUILD_GMOCK OFF"
    )

    include(GoogleTest)
       
    mrpc_add_test(
        TARGET test_proto_gen
        FILES "test/test_proto_gen.cpp"
        LIBRARIES mrpc
    )

    mrpc_add_test(
        TARGET test_dynamic_buffer
        FILES  "test/test_dynamic_buffer.cpp"
        LIBRARIES mrpc GTest::gtest_main
    )
    gtest_discover_tests(test_dynamic_buffer)
    
    mrpc_add_test(
        TARGET test_packet_handler
        FILES "test/test_packet_handler.cpp"
        LIBRARIES mrpc GTest::gtest_main
    )
endif()

