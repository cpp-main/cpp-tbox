cmake_minimum_required(VERSION 3.15)

set(TBOX_LOG_VERSION_MAJOR 0)
set(TBOX_LOG_VERSION_MINOR 0)
set(TBOX_LOG_VERSION_PATCH 1)
set(TBOX_LOG_VERSION ${TBOX_LOG_VERSION_MAJOR}.${TBOX_LOG_VERSION_MINOR}.${TBOX_LOG_VERSION_PATCH})

project(tbox_log)

add_definitions(-DLOG_MODULE_ID="tbox.log")

set(TBOX_LOG_HEADERS
    channel.h
    async_channel.h
    stdout_channel.h
    syslog_channel.h
    filelog_channel.h)

set(TBOX_LOG_SOURCES
    channel.cpp
    async_channel.cpp
    stdout_channel.cpp
    syslog_channel.cpp
    filelog_channel.cpp)

set(TBOX_LOG_TEST_SOURCES
    stdout_channel_test.cpp
    syslog_channel_test.cpp
    async_channel_test.cpp
    filelog_channel_test.cpp)

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
