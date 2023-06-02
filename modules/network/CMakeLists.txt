cmake_minimum_required(VERSION 3.15)

set(TBOX_NETWORK_VERSION_MAJOR 1)
set(TBOX_NETWORK_VERSION_MINOR 0)
set(TBOX_NETWORK_VERSION_PATCH 0)
set(TBOX_NETWORK_VERSION ${TBOX_NETWORK_VERSION_MAJOR}.${TBOX_NETWORK_VERSION_MINOR}.${TBOX_NETWORK_VERSION_PATCH})

project(tbox_network)

add_definitions(-DLOG_MODULE_ID="tbox.network")

set(TBOX_NETWORK_HEADERS
    buffer.h
    fd.h
    buffered_fd.h
    byte_stream.h
    stdio_stream.h
    uart.h
    socket_fd.h
    ip_address.h
    sockaddr.h
    udp_socket.h
    tcp_connection.h
    tcp_acceptor.h
    tcp_connector.h
    tcp_client.h
    tcp_server.h
    net_if.h
    domain_name.h
    dns_request.h)

set(TBOX_NETWORK_SOURCES
    buffer.cpp
    fd.cpp
    buffered_fd.cpp
    stdio_stream.cpp
    uart.cpp
    socket_fd.cpp
    ip_address.cpp
    sockaddr.cpp
    udp_socket.cpp
    tcp_connection.cpp
    tcp_acceptor.cpp
    tcp_connector.cpp
    tcp_client.cpp
    tcp_server.cpp
    net_if.cpp
    dns_request.cpp)

set(TBOX_NETWORK_TEST_SOURCES
    fd_test.cpp
    buffer_test.cpp
    buffered_fd_test.cpp
    uart_test.cpp
    ip_address_test.cpp
    sockaddr_test.cpp
    udp_socket_test.cpp
    net_if_test.cpp
    dns_request_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_NETWORK_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_NETWORK_VERSION}
                      SOVERSION ${TBOX_NETWORK_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_NETWORK_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_util tbox_event tbox_eventx rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_NETWORK_HEADERS} DESTINATION include/tbox/network)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
