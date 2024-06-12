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

set(TBOX_EVENTX_VERSION_MAJOR 1)
set(TBOX_EVENTX_VERSION_MINOR 0)
set(TBOX_EVENTX_VERSION_PATCH 2)
set(TBOX_EVENTX_VERSION ${TBOX_EVENTX_VERSION_MAJOR}.${TBOX_EVENTX_VERSION_MINOR}.${TBOX_EVENTX_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.eventx")

set(TBOX_LIBRARY_NAME tbox_eventx)

set(TBOX_EVENTX_HEADERS
    thread_pool.h
    timer_pool.h
    timeout_monitor.hpp
    timeout_monitor_impl.hpp
    request_pool.hpp
    loop_wdog.h
    work_thread.h
    loop_thread.h
    timer_fd.h
    async.h)

set(TBOX_EVENTX_SOURCES
    thread_pool.cpp
    timer_pool.cpp
    loop_wdog.cpp
    work_thread.cpp
    loop_thread.cpp
    timer_fd.cpp
    async.cpp)

set(TBOX_EVENTX_TEST_SOURCES
    thread_pool_test.cpp
    timer_pool_test.cpp
    timeout_monitor_test.cpp
    request_pool_test.cpp
    loop_wdog_test.cpp
    work_thread_test.cpp
    loop_thread_test.cpp
    timer_fd_test.cpp
    async_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_EVENTX_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_EVENTX_VERSION}
    SOVERSION ${TBOX_EVENTX_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_EVENTX_TEST_SOURCES})
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
    FILES ${TBOX_EVENTX_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/eventx
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
