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

include(CheckSymbolExists)

set(TBOX_BASE_VERSION_MAJOR 1)
set(TBOX_BASE_VERSION_MINOR 0)
set(TBOX_BASE_VERSION_PATCH 1)
set(TBOX_BASE_VERSION ${TBOX_BASE_VERSION_MAJOR}.${TBOX_BASE_VERSION_MINOR}.${TBOX_BASE_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.base")

set(TBOX_LIBRARY_NAME tbox_base)

set(TBOX_BASE_HEADERS
    version.h
    log.h
    log_impl.h
    log_output.h
    defines.h
    scope_exit.hpp
    cabinet.hpp
    cabinet_token.h
    memblock.h
    json_fwd.h
    json.hpp
    assert.h
    lifetime_tag.hpp
    backtrace.h
    catch_throw.h
    func_types.h
    object_pool.hpp
    recorder.h
    wrapped_recorder.h)

set(TBOX_BASE_SOURCES
    version.cpp
    log_impl.cpp
    log_output.cpp
    backtrace.cpp
    catch_throw.cpp
    recorder.cpp)

set(TBOX_BASE_TEST_SOURCES
    log_output_test.cpp
    scope_exit_test.cpp
    cabinet_token_test.cpp
    cabinet_test.cpp
    json_test.cpp
    lifetime_tag_test.cpp
    backtrace_test.cpp
    catch_throw_test.cpp
    object_pool_test.cpp
    recorder_test.cpp)

check_symbol_exists(backtrace "execinfo.h" HAVE_EXECINFO_H)

if (HAVE_EXECINFO_H)
    add_definitions(-DHAVE_EXECINFO_H=1)
else()
    message(WARNING "not support execinfo.h, can't use ::backtrace()")
    add_definitions(-DHAVE_EXECINFO_H=0)
endif()

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_BASE_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_BASE_VERSION}
    SOVERSION ${TBOX_BASE_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_BASE_TEST_SOURCES})

    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} rt dl)
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
    FILES ${TBOX_BASE_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/base
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
