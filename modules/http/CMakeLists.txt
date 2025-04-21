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

set(TBOX_HTTP_VERSION_MAJOR 0)
set(TBOX_HTTP_VERSION_MINOR 0)
set(TBOX_HTTP_VERSION_PATCH 1)
set(TBOX_HTTP_VERSION ${TBOX_HTTP_VERSION_MAJOR}.${TBOX_HTTP_VERSION_MINOR}.${TBOX_HTTP_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.http")

set(TBOX_LIBRARY_NAME tbox_http)

set(TBOX_HTTP_SOURCES
    common.cpp
    request.cpp
    respond.cpp
    url.cpp
    server/request_parser.cpp
    server/server.cpp
    server/server_imp.cpp
    server/context.cpp
    server/middlewares/router_middleware.cpp
    server/middlewares/form_data.cpp
    server/middlewares/form_data_middleware.cpp
    server/middlewares/file_downloader_middleware.cpp
    client/client.cpp)

set(TBOX_HTTP_TEST_SOURCES
    common_test.cpp
    respond_test.cpp
    request_test.cpp
    url_test.cpp
    server/request_parser_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_HTTP_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_HTTP_VERSION}
    SOVERSION ${TBOX_HTTP_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_HTTP_TEST_SOURCES})
    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_base tbox_network tbox_log tbox_event tbox_util rt dl)
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
    common.h
    request.h
    respond.h
    url.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/http
)

install(
    FILES
    server/types.h
    server/server.h
    server/context.h
    server/middleware.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/http/server
)

install(
    FILES
    server/middlewares/router_middleware.h
    server/middlewares/form_data.h
    server/middlewares/form_data_middleware.h
    server/middlewares/file_downloader_middleware.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/http/server/middlewares
)

install(
    FILES
    client/client.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/http/client
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
