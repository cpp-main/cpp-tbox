cmake_minimum_required(VERSION 3.15)

set(TBOX_HTTP_VERSION_MAJOR 0)
set(TBOX_HTTP_VERSION_MINOR 0)
set(TBOX_HTTP_VERSION_PATCH 1)
set(TBOX_HTTP_VERSION ${TBOX_HTTP_VERSION_MAJOR}.${TBOX_HTTP_VERSION_MINOR}.${TBOX_HTTP_VERSION_PATCH})

project(tbox_http)

add_definitions(-DLOG_MODULE_ID="tbox.http")

set(TBOX_HTTP_HEADERS
    common.h
    request.h
    respond.h
    url.h
    server/types.h
    server/server.h
    server/context.h
    server/middleware.h
    server/router.h
    client/client.h)

set(TBOX_HTTP_SOURCES
    common.cpp
    request.cpp
    respond.cpp
    url.cpp
    server/request_parser.cpp
    server/server.cpp
    server/server_imp.cpp
    server/context.cpp
    server/router.cpp
    client/client.cpp)

set(TBOX_HTTP_TEST_SOURCES
    common_test.cpp
    respond_test.cpp
    request_test.cpp
    url_test.cpp
    server/request_parser_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_HTTP_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_HTTP_VERSION}
                      SOVERSION ${TBOX_HTTP_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_HTTP_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_network tbox_log tbox_event tbox_util rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_HTTP_HEADERS} DESTINATION include/tbox/http)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
