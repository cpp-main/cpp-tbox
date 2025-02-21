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

set(TBOX_NETWORK_VERSION_MAJOR 1)
set(TBOX_NETWORK_VERSION_MINOR 0)
set(TBOX_NETWORK_VERSION_PATCH 0)
set(TBOX_NETWORK_VERSION ${TBOX_NETWORK_VERSION_MAJOR}.${TBOX_NETWORK_VERSION_MINOR}.${TBOX_NETWORK_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.network")

set(TBOX_LIBRARY_NAME tbox_network)

set(TBOX_NETWORK_HEADERS
    buffered_fd.h
    byte_stream.h
    stdio_stream.h
    uart.h
    socket_fd.h
    ip_address.h
    sockaddr.h
    udp_socket.h
    tcp_connection.h
    tcp_acceptor.h
    tcp_connector.h
    tcp_client.h
    tcp_server.h
    net_if.h
    domain_name.h
    dns_request.h)

set(TBOX_NETWORK_SOURCES
    buffered_fd.cpp
    stdio_stream.cpp
    uart.cpp
    socket_fd.cpp
    ip_address.cpp
    sockaddr.cpp
    udp_socket.cpp
    tcp_connection.cpp
    tcp_acceptor.cpp
    tcp_connector.cpp
    tcp_client.cpp
    tcp_server.cpp
    net_if.cpp
    dns_request.cpp)

set(TBOX_NETWORK_TEST_SOURCES
    buffered_fd_test.cpp
    uart_test.cpp
    ip_address_test.cpp
    sockaddr_test.cpp
    udp_socket_test.cpp
    net_if_test.cpp
    dns_request_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_NETWORK_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_NETWORK_VERSION}
    SOVERSION ${TBOX_NETWORK_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_NETWORK_TEST_SOURCES})
    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_base tbox_util tbox_event tbox_eventx rt dl)
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
    FILES ${TBOX_NETWORK_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/network
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
