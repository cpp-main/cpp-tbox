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

set(TBOX_COROUTINE_VERSION_MAJOR 0)
set(TBOX_COROUTINE_VERSION_MINOR 0)
set(TBOX_COROUTINE_VERSION_PATCH 1)
set(TBOX_COROUTINE_VERSION ${TBOX_COROUTINE_VERSION_MAJOR}.${TBOX_COROUTINE_VERSION_MINOR}.${TBOX_COROUTINE_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.coroutine")
set(TBOX_LIBRARY_NAME tbox_coroutine)

set(TBOX_COROUTINE_HEADERS
    scheduler.h
    channel.hpp
    semaphore.hpp
    mutex.hpp
    broadcast.hpp
    condition.hpp)

set(TBOX_COROUTINE_SOURCES
    scheduler.cpp)

set(TBOX_COROUTINE_TEST_SOURCES
    scheduler_test.cpp
    channel_test.cpp
    semaphore_test.cpp
    mutex_test.cpp
    broadcast_test.cpp
    condition_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_COROUTINE_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(${TBOX_LIBRARY_NAME} PROPERTIES
                        VERSION ${TBOX_COROUTINE_VERSION}
                      SOVERSION ${TBOX_COROUTINE_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_COROUTINE_TEST_SOURCES})
    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_base tbox_event rt dl)
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
    FILES ${TBOX_COROUTINE_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/coroutine
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
