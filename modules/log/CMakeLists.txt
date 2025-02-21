#
#     .============.
#    //  M A K E  / \
#   //  C++ DEV  /   \
#  //  E A S Y  /  \/ \
# ++ ----------.  \/\  .
#  \\     \     \ /\  /
#   \\     \     \   /
#    \\     \     \ /
#     -============'
#
# Copyright (c) 2018 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

cmake_minimum_required(VERSION 3.15)

set(TBOX_LOG_VERSION_MAJOR 0)
set(TBOX_LOG_VERSION_MINOR 0)
set(TBOX_LOG_VERSION_PATCH 1)
set(TBOX_LOG_VERSION ${TBOX_LOG_VERSION_MAJOR}.${TBOX_LOG_VERSION_MINOR}.${TBOX_LOG_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.log")

set(TBOX_LIBRARY_NAME tbox_log)

set(TBOX_LOG_HEADERS
    sink.h
    sync_stdout_sink.h
    async_sink.h
    async_stdout_sink.h
    async_syslog_sink.h
    async_file_sink.h)

set(TBOX_LOG_SOURCES
    sink.cpp
    sync_stdout_sink.cpp
    async_sink.cpp
    async_stdout_sink.cpp
    async_syslog_sink.cpp
    async_file_sink.cpp)

set(TBOX_LOG_TEST_SOURCES
    sync_stdout_sink_test.cpp
    async_sink_test.cpp
    async_stdout_sink_test.cpp
    async_syslog_sink_test.cpp
    async_file_sink_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_LOG_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_LOG_VERSION}
    SOVERSION ${TBOX_LOG_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_LOG_TEST_SOURCES})
    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_base tbox_util tbox_event rt dl)
    add_test(NAME ${TBOX_LIBRARY_NAME}_test COMMAND ${TBOX_LIBRARY_NAME}_test)
endif()

# install the target and create export-set
install(
    TARGETS ${TBOX_LIBRARY_NAME}
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install header file
install(
    FILES ${TBOX_LOG_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/log
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
