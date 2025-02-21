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

set(TBOX_CRYPTO_VERSION_MAJOR 0)
set(TBOX_CRYPTO_VERSION_MINOR 0)
set(TBOX_CRYPTO_VERSION_PATCH 1)
set(TBOX_CRYPTO_VERSION ${TBOX_CRYPTO_VERSION_MAJOR}.${TBOX_CRYPTO_VERSION_MINOR}.${TBOX_CRYPTO_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.crypto")

set(TBOX_LIBRARY_NAME tbox_crypto)

set(TBOX_CRYPTO_HEADERS
    md5.h
    aes.h)

set(TBOX_CRYPTO_SOURCES
    md5.cpp
    aes.cpp)

set(TBOX_CRYPTO_TEST_SOURCES
    md5_test.cpp
    aes_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_CRYPTO_SOURCES})

set_target_properties(${TBOX_LIBRARY_NAME} PROPERTIES
                        VERSION ${TBOX_CRYPTO_VERSION}
                      SOVERSION ${TBOX_CRYPTO_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_CRYPTO_TEST_SOURCES})
    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_util tbox_base rt dl)
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
    FILES ${TBOX_CRYPTO_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/crypto
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
