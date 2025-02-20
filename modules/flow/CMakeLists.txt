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

set(TBOX_FLOW_VERSION_MAJOR 0)
set(TBOX_FLOW_VERSION_MINOR 0)
set(TBOX_FLOW_VERSION_PATCH 3)
set(TBOX_FLOW_VERSION ${TBOX_FLOW_VERSION_MAJOR}.${TBOX_FLOW_VERSION_MINOR}.${TBOX_FLOW_VERSION_PATCH})

add_definitions(-DMODULE_ID="tbox.flow")

set(TBOX_LIBRARY_NAME tbox_flow)

set(TBOX_FLOW_SOURCES
    state_machine.cpp
    action.cpp
    action_executor.cpp
    event_publisher_impl.cpp
    actions/assemble_action.cpp
    actions/function_action.cpp
    actions/sleep_action.cpp
    actions/sequence_action.cpp
    actions/parallel_action.cpp
    actions/if_else_action.cpp
    actions/if_then_action.cpp
    actions/loop_action.cpp
    actions/loop_if_action.cpp
    actions/event_action.cpp
    actions/repeat_action.cpp
    actions/composite_action.cpp
    actions/wrapper_action.cpp
    actions/dummy_action.cpp
    actions/switch_action.cpp
    to_graphviz.cpp)

set(TBOX_FLOW_TEST_SOURCES
    state_machine_test.cpp
    action_test.cpp
    event_publisher_impl_test.cpp
    action_executor_test.cpp
    actions/assemble_action_test.cpp
    actions/sleep_action_test.cpp
    actions/function_action_test.cpp
    actions/sequence_action_test.cpp
    actions/parallel_action_test.cpp
    actions/if_else_action_test.cpp
    actions/if_then_action_test.cpp
    actions/loop_action_test.cpp
    actions/loop_if_action_test.cpp
    actions/repeat_action_test.cpp
    actions/composite_action_test.cpp
    actions/wrapper_action_test.cpp
    actions/succ_fail_action_test.cpp
    actions/switch_action_test.cpp
    to_graphviz_test.cpp)

add_library(${TBOX_LIBRARY_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_FLOW_SOURCES})
add_library(tbox::${TBOX_LIBRARY_NAME} ALIAS ${TBOX_LIBRARY_NAME})

set_target_properties(
    ${TBOX_LIBRARY_NAME} PROPERTIES
    VERSION ${TBOX_FLOW_VERSION}
    SOVERSION ${TBOX_FLOW_VERSION_MAJOR}
)

if(${TBOX_ENABLE_TEST})
    add_executable(${TBOX_LIBRARY_NAME}_test ${TBOX_FLOW_TEST_SOURCES})
    target_link_libraries(${TBOX_LIBRARY_NAME}_test gmock_main gmock gtest pthread ${TBOX_LIBRARY_NAME} tbox_eventx tbox_event tbox_util tbox_base rt dl)
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
    state_machine.h
    action.h
    action_reason.h
    event.h
    action_executor.h
    event_subscriber.h
    event_publisher.h
    event_publisher_impl.h
    to_graphviz.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/flow
)

install(
    FILES
    actions/assemble_action.h
    actions/function_action.h
    actions/sleep_action.h
    actions/sequence_action.h
    actions/parallel_action.h
    actions/if_else_action.h
    actions/if_then_action.h
    actions/loop_action.h
    actions/loop_if_action.h
    actions/event_action.h
    actions/repeat_action.h
    actions/composite_action.h
    actions/wrapper_action.h
    actions/succ_fail_action.h
    actions/dummy_action.h
    actions/switch_action.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tbox/flow/actions
)

# generate and install export file
install(
    EXPORT ${TBOX_LIBRARY_NAME}_targets
    FILE ${TBOX_LIBRARY_NAME}_targets.cmake
    NAMESPACE tbox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tbox
)
