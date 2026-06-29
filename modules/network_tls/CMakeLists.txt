#
#     .============.
#    //  M A K E  / \
#   //  C++ DEV  /   \
#   //  E A S E  /  \/ \
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

set(TBOX_NETWORK_TLS_VERSION_MAJOR 0)
set(TBOX_NETWORK_TLS_VERSION_MINOR 0)
set(TBOX_NETWORK_TLS_VERSION_PATCH 1)
set(TBOX_NETWORK_TLS_VERSION ${TBOX_NETWORK_TLS_VERSION_MAJOR}.${TBOX_NETWORK_TLS_VERSION_MINOR}.${TBOX_NETWORK_TLS_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.network_tls")

find_package(OpenSSL REQUIRED)

set(TBOX_LIBRARY_NAME tbox_network_tls)

set(TBOX_NETWORK_TLS_HEADERS
    buffered_ssl_fd.h
    tcp_tls_connection.h
    tcp_tls_acceptor.h
    tcp_tls_connector.h
    tcp_tls_factory.h)

set(TBOX_NETWORK_TLS_SOURCES
    tls_factory_entry.cpp
    buffered_ssl_fd.cpp
    tcp_tls_connection.cpp
    tcp_tls_acceptor.cpp
    tcp_tls_connector.cpp
    tcp_tls_factory.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_NETWORK_TLS_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_NETWORK_TLS_VERSION}
    SOVERSION ${TBOX_NETWORK_TLS_VERSION_MAJOR}
)

target_link_libraries(${TBOX_LIBRARY_NAME} tbox_network OpenSSL::SSL OpenSSL::Crypto)

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
    FILES ${TBOX_NETWORK_TLS_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/network
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
