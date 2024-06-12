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
# Copyright (c) 2024 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

cmake_minimum_required(VERSION 3.15)

set(TBOX_DBUS_VERSION_MAJOR 0)
set(TBOX_DBUS_VERSION_MINOR 0)
set(TBOX_DBUS_VERSION_PATCH 1)
set(TBOX_DBUS_VERSION ${TBOX_DBUS_VERSION_MAJOR}.${TBOX_DBUS_VERSION_MINOR}.${TBOX_DBUS_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.dbus")

set(TBOX_LIBRARY_NAME tbox_dbus)

set(TBOX_DBUS_HEADERS loop.h connection.h)
set(TBOX_DBUS_SOURCES loop.cpp connection.cpp)

find_package(DBus1 REQUIRED)
include_directories(${DBus1_INCLUDE_DIRS})

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_DBUS_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_DBUS_VERSION}
    SOVERSION ${TBOX_DBUS_VERSION_MAJOR}
)

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
    FILES ${TBOX_DBUS_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/dbus
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
