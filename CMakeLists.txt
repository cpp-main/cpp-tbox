#
# cpp-tbox
#
cmake_minimum_required(VERSION 3.10)

#
# Building in-tree is not allowed (we take care of your craziness).
#
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#
set(TBOX_VERSION_MAJOR 2)
set(TBOX_VERSION_MINOR 1)
set(TBOX_VERSION_REVISION 0)
set(TBOX_VERSION ${TBOX_VERSION_MAJOR}.${TBOX_VERSION_MINOR}.${TBOX_VERSION_REVISION})

#
# Set install prefix
#
#set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "")


project(
    tbox
    VERSION ${TBOX_VERSION}
    DESCRIPTION "Easy to use, convenient and modern C++ treasure box!"
    HOMEPAGE_URL "https://gitee.com/cpp-master/cpp-tbox"
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(TBOX_BUILD_LIB_TYPE SHARED)

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
option(TBOX_ENABLE_MAIN "build main" ON)

option(TBOX_ENABLE_COROUTINE "build coroutine" ON)
option(TBOX_ENABLE_HTTP "build http" ON)
option(TBOX_ENABLE_MQTT "build mqtt" ON)
option(TBOX_ENABLE_FLOW "build flow" ON)
option(TBOX_ENABLE_ALARM "build alarm" ON)
option(TBOX_ENABLE_CRYPTO "build crypto" ON)

#
# TESTS
#
option(CMAKE_ENABLE_TEST "Whether to enable unit tests" OFF)

if(CMAKE_ENABLE_TEST)
    message(STATUS "Unit tests enabled")
    find_package(GTest REQUIRED)
    set(TBOX_ENABLE_TEST ON)
    enable_testing()
endif()


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

if(TBOX_ENABLE_BASE)
    message(STATUS "base module enabled")
    add_subdirectory(modules/base)
endif()

if(TBOX_ENABLE_UTIL)
    message(STATUS "util module enabled")
    add_subdirectory(modules/util)
endif()

if(TBOX_ENABLE_EVENT)
    message(STATUS "event module enabled")
    add_subdirectory(modules/event)
endif()

if(TBOX_ENABLE_EVENTX)
    message(STATUS "eventx module enabled")
    add_subdirectory(modules/eventx)
endif()

if(TBOX_ENABLE_LOG)
    message(STATUS "log module enabled")
    add_subdirectory(modules/log)
endif()

if(TBOX_ENABLE_NETWORK)
    message(STATUS "network module enabled")
    add_subdirectory(modules/network)
endif()

if(TBOX_ENABLE_TERMINAL)
    message(STATUS "terminal module enabled")
    add_subdirectory(modules/terminal)
endif()

if(TBOX_ENABLE_MAIN)
    message(STATUS "main module enabled")
    add_subdirectory(modules/main)
endif()

if(TBOX_ENABLE_COROUTINE)
    message(STATUS "coroutine module enabled")
    add_subdirectory(modules/coroutine)
endif()

if(TBOX_ENABLE_HTTP)
    message(STATUS "http module enabled")
    add_subdirectory(modules/http)
endif()

if(TBOX_ENABLE_MQTT)
    message(STATUS "mqtt module enabled")
    add_subdirectory(modules/mqtt)
endif()

if(TBOX_ENABLE_FLOW)
    message(STATUS "flow module enabled")
    add_subdirectory(modules/flow)
endif()

if(TBOX_ENABLE_ALARM)
    message(STATUS "alarm module enabled")
    add_subdirectory(modules/alarm)
endif()

if(TBOX_ENABLE_CRYPTO)
    message(STATUS "crypto module enabled")
    add_subdirectory(modules/crypto)
endif()

