cmake_minimum_required(VERSION 3.15)

set(TBOX_FLOW_VERSION_MAJOR 0)
set(TBOX_FLOW_VERSION_MINOR 0)
set(TBOX_FLOW_VERSION_PATCH 3)
set(TBOX_FLOW_VERSION ${TBOX_FLOW_VERSION_MAJOR}.${TBOX_FLOW_VERSION_MINOR}.${TBOX_FLOW_VERSION_PATCH})

project(tbox_flow)

add_definitions(-DLOG_MODULE_ID="tbox.flow")

set(TBOX_FLOW_HEADERS
    state_machine.h
    action.h
    event.h
    action_executor.h
    event_subscriber.h
    event_publisher.h
    event_publisher_impl.h
    actions/function_action.h
    actions/sleep_action.h
    actions/sequence_action.h
    actions/parallel_action.h
    actions/if_else_action.h
    actions/loop_action.h
    actions/loop_if_action.h
    actions/event_action.h
    actions/repeat_action.h
    actions/composite_action.h
    actions/wrapper_action.h
    actions/succ_fail_action.h
    to_graphviz.h)

set(TBOX_FLOW_SOURCES
    state_machine.cpp
    action.cpp
    action_executor.cpp
    event_publisher_impl.cpp
    actions/function_action.cpp
    actions/sleep_action.cpp
    actions/sequence_action.cpp
    actions/parallel_action.cpp
    actions/if_else_action.cpp
    actions/loop_action.cpp
    actions/loop_if_action.cpp
    actions/event_action.cpp
    actions/repeat_action.cpp
    actions/composite_action.cpp
    actions/wrapper_action.cpp
    to_graphviz.cpp)

set(TBOX_FLOW_TEST_SOURCES
    state_machine_test.cpp
    action_test.cpp
    event_publisher_impl_test.cpp
    action_executor_test.cpp
    actions/sleep_action_test.cpp
    actions/function_action_test.cpp
    actions/sequence_action_test.cpp
    actions/parallel_action_test.cpp
    actions/if_else_action_test.cpp
    actions/loop_action_test.cpp
    actions/loop_if_action_test.cpp
    actions/repeat_action_test.cpp
    actions/composite_action_test.cpp
    actions/wrapper_action_test.cpp
    actions/succ_fail_action_test.cpp
    to_graphviz_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_FLOW_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_FLOW_VERSION}
                      SOVERSION ${TBOX_FLOW_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_FLOW_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_event tbox_util rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_FLOW_HEADERS} DESTINATION include/tbox/flow)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
