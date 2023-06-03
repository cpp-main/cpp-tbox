cmake_minimum_required(VERSION 3.15)

set(TBOX_LOG_VERSION_MAJOR 0)
set(TBOX_LOG_VERSION_MINOR 0)
set(TBOX_LOG_VERSION_PATCH 1)
set(TBOX_LOG_VERSION ${TBOX_LOG_VERSION_MAJOR}.${TBOX_LOG_VERSION_MINOR}.${TBOX_LOG_VERSION_PATCH})

project(tbox_log)

add_definitions(-DLOG_MODULE_ID="tbox.log")

set(TBOX_LOG_HEADERS
    sink.h
    async_sink.h
    async_stdout_sink.h
    async_syslog_sink.h
    async_file_sink.h)

set(TBOX_LOG_SOURCES
    sink.cpp
    async_sink.cpp
    async_stdout_sink.cpp
    async_syslog_sink.cpp
    async_file_sink.cpp)

set(TBOX_LOG_TEST_SOURCES
    async_sink_test.cpp
    async_stdout_sink_test.cpp
    async_syslog_sink_test.cpp
    async_file_sink_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_LOG_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_LOG_VERSION}
                      SOVERSION ${TBOX_LOG_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_LOG_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_util tbox_event rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_LOG_HEADERS} DESTINATION include/tbox/log)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
