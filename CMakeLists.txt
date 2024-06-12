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

#
# Building in-tree is not allowed (we take care of your craziness).
#
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#
file(STRINGS "version.mk" VERSION_CONTENTS)
set(REGEX_MAJOR "TBOX_VERSION_MAJOR *:= *([0-9]+)")
set(REGEX_MINOR "TBOX_VERSION_MINOR *:= *([0-9]+)")
set(REGEX_REVISION "TBOX_VERSION_REVISION *:= *([0-9]+)")
string(REGEX MATCH "${REGEX_MAJOR}" _ ${VERSION_CONTENTS})
set(TBOX_VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "${REGEX_MINOR}" _ ${VERSION_CONTENTS})
set(TBOX_VERSION_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "${REGEX_REVISION}" _ ${VERSION_CONTENTS})
set(TBOX_VERSION_REVISION ${CMAKE_MATCH_1})

set(TBOX_VERSION ${TBOX_VERSION_MAJOR}.${TBOX_VERSION_MINOR}.${TBOX_VERSION_REVISION})



#
# Set install prefix
#
#set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "")

project(
    cpp-tbox
    VERSION ${TBOX_VERSION}
    DESCRIPTION "Easy to use, convenient and modern C++ treasure box!"
    HOMEPAGE_URL "https://gitee.com/cpp-master/cpp-tbox"
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(TBOX_BUILD_LIB_TYPE STATIC)

message("*")
message("* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message("* Copyright (c) 2020-2025 Hevake,Lucas")
message("*")

option(TBOX_ENABLE_BASE "build base" ON)
option(TBOX_ENABLE_UTIL "build util" ON)
option(TBOX_ENABLE_EVENT "build event" ON)
option(TBOX_ENABLE_EVENTX "build eventx" ON)
option(TBOX_ENABLE_LOG "build log" ON)
option(TBOX_ENABLE_NETWORK "build network" ON)
option(TBOX_ENABLE_TERMINAL "build terminal" ON)
option(TBOX_ENABLE_TRACE "build trace" ON)
option(TBOX_ENABLE_MAIN "build main" ON)

option(TBOX_ENABLE_COROUTINE "build coroutine" ON)
option(TBOX_ENABLE_HTTP "build http" ON)
option(TBOX_ENABLE_MQTT "build mqtt" ON)
option(TBOX_ENABLE_FLOW "build flow" ON)
option(TBOX_ENABLE_ALARM "build alarm" ON)
option(TBOX_ENABLE_CRYPTO "build crypto" ON)
option(TBOX_ENABLE_JSONRPC "build jsonrpc" ON)
option(TBOX_ENABLE_DBUS "build dbus" ON)

#
# 3rd-party libraries
#
option(TBOX_ENABLE_NLOHMANN_JSON "install nlohmann/json" ON)

#
# TESTS
#
option(CMAKE_ENABLE_TEST "Whether to enable unit tests" ON)

if(CMAKE_ENABLE_TEST)
    message(STATUS "Unit tests enabled")
    find_package(GTest REQUIRED)
    set(TBOX_ENABLE_TEST ON)
    enable_testing()
endif()

include(GNUInstallDirs)

#
# Compiler args
#
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -Wextra -Werror -Wno-missing-field-initializers)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd-party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules)

add_definitions(-DTBOX_VERSION_MAJOR=${TBOX_VERSION_MAJOR}
                -DTBOX_VERSION_MINOR=${TBOX_VERSION_MINOR}
                -DTBOX_VERSION_REVISION=${TBOX_VERSION_REVISION})

set(TBOX_COMPONENTS)

if(TBOX_ENABLE_BASE)
    message(STATUS "base module enabled")
    list(APPEND TBOX_COMPONENTS base)
endif()

if(TBOX_ENABLE_UTIL)
    message(STATUS "util module enabled")
    list(APPEND TBOX_COMPONENTS util)
endif()

if(TBOX_ENABLE_EVENT)
    message(STATUS "event module enabled")
    list(APPEND TBOX_COMPONENTS event)
endif()

if(TBOX_ENABLE_EVENTX)
    message(STATUS "eventx module enabled")
    list(APPEND TBOX_COMPONENTS eventx)
endif()

if(TBOX_ENABLE_LOG)
    message(STATUS "log module enabled")
    list(APPEND TBOX_COMPONENTS log)
endif()

if(TBOX_ENABLE_NETWORK)
    message(STATUS "network module enabled")
    list(APPEND TBOX_COMPONENTS network)
endif()

if(TBOX_ENABLE_TERMINAL)
    message(STATUS "terminal module enabled")
    list(APPEND TBOX_COMPONENTS terminal)
endif()

if(TBOX_ENABLE_TRACE)
    message(STATUS "trace module enabled")
    list(APPEND TBOX_COMPONENTS trace)
endif()

if(TBOX_ENABLE_MAIN)
    message(STATUS "main module enabled")
    list(APPEND TBOX_COMPONENTS main)
endif()

if(TBOX_ENABLE_COROUTINE)
    message(STATUS "coroutine module enabled")
    list(APPEND TBOX_COMPONENTS coroutine)
endif()

if(TBOX_ENABLE_HTTP)
    message(STATUS "http module enabled")
    list(APPEND TBOX_COMPONENTS http)
endif()

if(TBOX_ENABLE_MQTT)
    message(STATUS "mqtt module enabled")
    list(APPEND TBOX_COMPONENTS mqtt)
endif()

if(TBOX_ENABLE_FLOW)
    message(STATUS "flow module enabled")
    list(APPEND TBOX_COMPONENTS flow)
endif()

if(TBOX_ENABLE_ALARM)
    message(STATUS "alarm module enabled")
    list(APPEND TBOX_COMPONENTS alarm)
endif()

if(TBOX_ENABLE_CRYPTO)
    message(STATUS "crypto module enabled")
    list(APPEND TBOX_COMPONENTS crypto)
endif()

if(TBOX_ENABLE_JSONRPC)
    message(STATUS "jsonrpc module enabled")
    list(APPEND TBOX_COMPONENTS jsonrpc)
endif()

if(TBOX_ENABLE_DBUS)
    message(STATUS "dbus module enabled")
    list(APPEND TBOX_COMPONENTS dbus)
endif()

foreach(item IN LISTS TBOX_COMPONENTS)
    add_subdirectory(modules/${item})
endforeach()

if(TBOX_ENABLE_NLOHMANN_JSON)
    message(STATUS "nlohmann/json installation enabled")
    list(APPEND TBOX_COMPONENTS nlohmann)
    add_subdirectory(3rd-party/nlohmann)
endif()

#
# installation
#
include(CMakePackageConfigHelpers)

# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/tboxConfigVersion.cmake"
  VERSION "${TBOX_VERSION}"
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/tboxConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
    PATH_VARS TBOX_COMPONENTS
)

install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/tboxConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/tboxConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
