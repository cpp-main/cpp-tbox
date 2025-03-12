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

include(CheckSymbolExists)

set(TBOX_EVENT_VERSION_MAJOR 1)
set(TBOX_EVENT_VERSION_MINOR 1)
set(TBOX_EVENT_VERSION_PATCH 2)
set(TBOX_EVENT_VERSION ${TBOX_EVENT_VERSION_MAJOR}.${TBOX_EVENT_VERSION_MINOR}.${TBOX_EVENT_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.event")

set(TBOX_LIBRARY_NAME tbox_event)

set(TBOX_EVENT_HEADERS
    forward.h
    loop.h
    event.h
    fd_event.h
    timer_event.h
    signal_event.h
    stat.h
)

set(TBOX_EVENT_SOURCES
    loop.cpp
    common_loop.cpp
    common_loop_timer.cpp
    common_loop_signal.cpp
    common_loop_run.cpp
    timer_event_impl.cpp
    signal_event_impl.cpp
    misc.cpp
    stat.cpp
)

set(TBOX_EVENT_TEST_SOURCES
    common_loop_test.cpp
    fd_event_test.cpp
    timer_event_test.cpp
    signal_event_test.cpp
)

check_symbol_exists(select "sys/select.h" HAVE_SELECT)
check_symbol_exists(epoll_create1 "sys/epoll.h" HAVE_EPOLL)

if (NOT HAVE_SELECT AND NOT HAVE_EPOLL)
    message(FATAL_ERROR "Neither epoll nor select is supported by the system. Project cannot proceed.")
endif()

if (HAVE_SELECT)
    add_definitions(-DHAVE_SELECT=1)
    list(APPEND TBOX_EVENT_SOURCES
        engines/select/loop.cpp
        engines/select/fd_event.cpp
    )
endif()

if (HAVE_EPOLL)
    add_definitions(-DHAVE_EPOLL=1)
    list(APPEND TBOX_EVENT_SOURCES
        engines/epoll/loop.cpp
        engines/epoll/fd_event.cpp
    )
endif()

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_EVENT_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_EVENT_VERSION}
    SOVERSION ${TBOX_EVENT_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_EVENT_TEST_SOURCES})

    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_base rt dl)
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
    FILES ${TBOX_EVENT_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/event
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
