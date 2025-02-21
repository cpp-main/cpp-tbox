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

set(TBOX_TERMINAL_VERSION_MAJOR 0)
set(TBOX_TERMINAL_VERSION_MINOR 0)
set(TBOX_TERMINAL_VERSION_PATCH 2)
set(TBOX_TERMINAL_VERSION ${TBOX_TERMINAL_VERSION_MAJOR}.${TBOX_TERMINAL_VERSION_MINOR}.${TBOX_TERMINAL_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.terminal")

set(TBOX_LIBRARY_NAME tbox_terminal)

set(TBOX_TERMINAL_SOURCES
    terminal.cpp
    session.cpp
	helper.cpp
    service/telnetd.cpp
    service/tcp_rpc.cpp
    impl/terminal.cpp
    impl/terminal_key_events.cpp
    impl/terminal_commands.cpp
    impl/terminal_nodes.cpp
    impl/key_event_scanner.cpp
    impl/dir_node.cpp
    impl/func_node.cpp
    impl/service/telnetd.cpp
    impl/service/tcp_rpc.cpp)

set(TBOX_TERMINAL_TEST_SOURCES
    impl/key_event_scanner_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_TERMINAL_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_TERMINAL_VERSION}
    SOVERSION ${TBOX_TERMINAL_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_TERMINAL_TEST_SOURCES})
    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_base tbox_util tbox_event tbox_network rt dl)
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
    FILES
    types.h
    connection.h
    terminal_interact.h
    terminal_nodes.h
    session.h
    terminal.h
    helper.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/terminal
)

install(
    FILES
    service/telnetd.h
    service/tcp_rpc.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/terminal/service
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
