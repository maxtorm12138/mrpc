
function(generate_proto)
    if (NOT TARGET protobuf::protoc OR NOT TARGET protobuf::libprotobuf)
        message(FATAL_ERROR "could not find protoc")
    endif()

    get_target_property(PROTOBUF_IMPORT_PATH protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
    foreach(IMPORT_PATH IN LISTS PROTOBUF_IMPORT_PATH)
        get_filename_component(IMPORT_PATH ${IMPORT_PATH} ABSOLUTE)
        file(TO_NATIVE_PATH ${IMPORT_PATH} IMPORT_PATH)
        string(CONCAT IMPORT_ARG "--proto_path=" "${IMPORT_PATH}")
        list(APPEND PROTOC_ARGS ${IMPORT_ARG})
    endforeach()

    cmake_parse_arguments(GENERATE_PROTO "" "GENERATE_PATH;FILE;OUTPUT_FILE" "IMPORT_PATH" ${ARGN})

    get_filename_component(GENERATE_PROTO_GENERATE_PATH ${GENERATE_PROTO_GENERATE_PATH} ABSOLUTE)
    file(TO_NATIVE_PATH ${GENERATE_PROTO_GENERATE_PATH} GENERATE_PROTO_GENERATE_PATH)

    get_filename_component(GENERATE_PROTO_FILE ${GENERATE_PROTO_FILE} ABSOLUTE)
    file(TO_NATIVE_PATH ${GENERATE_PROTO_FILE} GENERATE_PROTO_FILE)

    get_filename_component(GENERATE_PROTO_FILE_NAME ${GENERATE_PROTO_FILE} NAME)
    string(REPLACE ".proto" ".pb.cc" GENERATE_PROTO_CPP_SRC "${GENERATE_PROTO_GENERATE_PATH}/${GENERATE_PROTO_FILE_NAME}")
    string(REPLACE ".proto" ".pb.h" GENERATE_PROTO_CPP_HDR "${GENERATE_PROTO_GENERATE_PATH}/${GENERATE_PROTO_FILE_NAME}")
    file(TO_CMAKE_PATH ${GENERATE_PROTO_CPP_SRC} GENERATE_PROTO_CPP_SRC)
    file(TO_CMAKE_PATH ${GENERATE_PROTO_CPP_HDR} GENERATE_PROTO_CPP_HDR)

    file(MAKE_DIRECTORY ${GENERATE_PROTO_GENERATE_PATH})
        

    foreach(IMPORT_PATH ${GENERATE_PROTO_IMPORT_PATH})
        get_filename_component(IMPORT_PATH ${IMPORT_PATH} ABSOLUTE)
        file(TO_NATIVE_PATH ${IMPORT_PATH} IMPORT_PATH)
        string(CONCAT IMPORT_ARG "--proto_path=" "${IMPORT_PATH}")
        list(APPEND PROTOC_ARGS ${IMPORT_ARG})
    endforeach()
    list(APPEND PROTOC_ARGS "--cpp_out=${GENERATE_PROTO_GENERATE_PATH}" "${GENERATE_PROTO_FILE}")
    message("PROTOC_ARGS: ${PROTOC_ARGS}")

    add_custom_command(
        OUTPUT ${GENERATE_PROTO_CPP_SRC} ${GENERATE_PROTO_CPP_HDR}
        COMMAND protobuf::protoc ${PROTOC_ARGS}
        DEPENDS ${GENERATE_PROTO_FILE} protobuf::protoc
        VERBATIM)

    if (${RV})
        message(FATAL_ERROR "Generation of data model returned ${RV} for proto ${NATIVE_PROTO_FILE}")
    endif ()
    set(${GENERATE_PROTO_OUTPUT_FILE} ${GENERATE_PROTO_CPP_SRC} ${GENERATE_PROTO_CPP_HDR} PARENT_SCOPE)
    
endfunction()