cmake_minimum_required(VERSION 3.15)

set(TBOX_UTIL_VERSION_MAJOR 0)
set(TBOX_UTIL_VERSION_MINOR 0)
set(TBOX_UTIL_VERSION_PATCH 2)
set(TBOX_UTIL_VERSION ${TBOX_UTIL_VERSION_MAJOR}.${TBOX_UTIL_VERSION_MINOR}.${TBOX_UTIL_VERSION_PATCH})

project(tbox_util)

add_definitions(-DLOG_MODULE_ID="tbox.util")

set(TBOX_UTIL_HEADERS
    pid_file.h
    string.h
    fs.h
    argument_parser.h
    split_cmdline.h
    serializer.h
    time_counter.h
    async_pipe.h
    timestamp.h
    json.h
    base64.h
    checksum.h
    crc.h
    json_deep_loader.h
    execute_cmd.h)

set(TBOX_UTIL_SOURCES
    pid_file.cpp
    string.cpp
    fs.cpp
    argument_parser.cpp
    split_cmdline.cpp
    serializer.cpp
    time_counter.cpp
    async_pipe.cpp
    timestamp.cpp
    json.cpp
    base64.cpp
    checksum.cpp
    crc.cpp
    json_deep_loader.cpp
    execute_cmd.cpp)

set(TBOX_UTIL_TEST_SOURCES
    pid_file_test.cpp
    string_test.cpp
    fs_test.cpp
    argument_parser_test.cpp
    split_cmdline_test.cpp
    serializer_test.cpp
    time_counter_test.cpp
    async_pipe_test.cpp
    json_test.cpp
    base64_test.cpp
    checksum_test.cpp
    crc_test.cpp
    execute_cmd_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_UTIL_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_UTIL_VERSION}
                      SOVERSION ${TBOX_UTIL_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_UTIL_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_UTIL_HEADERS} DESTINATION include/tbox/util)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)