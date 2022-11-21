cmake_minimum_required(VERSION 3.14)
project(mrpc LANGUAGES CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

function(generate_proto PROTO_ROOT PROTO_FILE GENERATE_PATH OUT_FILES)
    get_filename_component(PROTO_FILE ${PROTO_FILE} ABSOLUTE)
    get_filename_component(PROTO_ROOT ${PROTO_ROOT} ABSOLUTE)
    get_filename_component(PROTO_FILE_NAME ${PROTO_FILE} NAME)

    string(REPLACE ".proto" ".pb.cc" PROTO_CPP_SRC "${GENERATE_PATH}/${PROTO_FILE_NAME}")
    string(REPLACE ".proto" ".pb.h" PROTO_CPP_HDR "${GENERATE_PATH}/${PROTO_FILE_NAME}")

    file(TO_NATIVE_PATH ${PROTO_FILE} NATIVE_PROTO_FILE)
    file(TO_NATIVE_PATH ${GENERATE_PATH} NATIVE_GENERATE_PATH)
    file(TO_NATIVE_PATH ${PROTO_ROOT} NATIVE_PROTO_ROOT)

    file(MAKE_DIRECTORY ${NATIVE_GENERATE_PATH})

    execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${NATIVE_PROTO_ROOT} --cpp_out=${NATIVE_GENERATE_PATH} ${NATIVE_PROTO_FILE} RESULT_VARIABLE RV)

    if (${RV})
        message(FATAL_ERROR "Generation of data model returned ${RV} for proto ${NATIVE_PROTO_FILE}")
    endif ()
    set(${OUT_FILES} ${PROTO_CPP_SRC} ${PROTO_CPP_HDR} PARENT_SCOPE)
endfunction()

generate_proto(proto/ proto/rpc.proto ${CMAKE_CURRENT_BINARY_DIR}/proto PROTO_FILE)

set(MRPC_PROTO_IMPORT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto)