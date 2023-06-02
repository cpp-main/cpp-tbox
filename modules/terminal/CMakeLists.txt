cmake_minimum_required(VERSION 3.15)

set(TBOX_TERMINAL_VERSION_MAJOR 0)
set(TBOX_TERMINAL_VERSION_MINOR 0)
set(TBOX_TERMINAL_VERSION_PATCH 2)
set(TBOX_TERMINAL_VERSION ${TBOX_TERMINAL_VERSION_MAJOR}.${TBOX_TERMINAL_VERSION_MINOR}.${TBOX_TERMINAL_VERSION_PATCH})

project(tbox_terminal)

add_definitions(-DLOG_MODULE_ID="tbox.terminal")

set(TBOX_TERMINAL_HEADERS
    types.h
    connection.h
    terminal_interact.h
    terminal_nodes.h
    session.h
    terminal.h
    service/telnetd.h
    service/tcp_rpc.h)

set(TBOX_TERMINAL_SOURCES
    terminal.cpp
    session.cpp
    service/telnetd.cpp
    service/tcp_rpc.cpp
    impl/terminal.cpp
    impl/terminal_key_events.cpp
    impl/terminal_commands.cpp
    impl/terminal_nodes.cpp
    impl/key_event_scanner.cpp
    impl/dir_node.cpp
    impl/func_node.cpp
    impl/service/telnetd.cpp
    impl/service/tcp_rpc.cpp)

set(TBOX_TERMINAL_TEST_SOURCES
    impl/key_event_scanner_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_TERMINAL_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_TERMINAL_VERSION}
                      SOVERSION ${TBOX_TERMINAL_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_TERMINAL_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_util tbox_event tbox_network rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_TERMINAL_HEADERS} DESTINATION include/tbox/terminal)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
