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

set(TBOX_JSONRPC_VERSION_MAJOR 0)
set(TBOX_JSONRPC_VERSION_MINOR 0)
set(TBOX_JSONRPC_VERSION_PATCH 1)
set(TBOX_JSONRPC_VERSION ${TBOX_JSONRPC_VERSION_MAJOR}.${TBOX_JSONRPC_VERSION_MINOR}.${TBOX_JSONRPC_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.jsonrpc")

set(TBOX_LIBRARY_NAME tbox_jsonrpc)

set(TBOX_JSONRPC_SOURCES
    proto.cpp
    protos/raw_stream_proto.cpp
    protos/header_stream_proto.cpp
    protos/packet_proto.cpp
    rpc.cpp)

set(TBOX_JSONRPC_TEST_SOURCES
    ${TBOX_JSONRPC_SOURCES}
    protos/raw_stream_proto_test.cpp
    protos/header_stream_proto_test.cpp
    protos/packet_proto_test.cpp
    rpc_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_JSONRPC_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_JSONRPC_VERSION}
    SOVERSION ${TBOX_JSONRPC_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_JSONRPC_TEST_SOURCES})

    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_base tbox_event tbox_util rt dl)
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
    FILES proto.h rpc.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/jsonrpc
)

install(
    FILES
    protos/raw_stream_proto.h
    protos/header_stream_proto.h
    protos/packet_proto.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/jsonrpc/protos
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
