cmake_minimum_required(VERSION 3.15)

set(TBOX_ASYNC_VERSION_MAJOR 0)
set(TBOX_ASYNC_VERSION_MINOR 0)
set(TBOX_ASYNC_VERSION_PATCH 1)
set(TBOX_ASYNC_VERSION ${TBOX_ASYNC_VERSION_MAJOR}.${TBOX_ASYNC_VERSION_MINOR}.${TBOX_ASYNC_VERSION_PATCH})

project(tbox_async)

add_definitions(-DLOG_MODULE_ID="tbox.async")

set(TBOX_ASYNC_HEADERS async.h)

set(TBOX_ASYNC_SOURCES async.cpp)

set(TBOX_ASYNC_TEST_SOURCES async_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_ASYNC_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_ASYNC_VERSION}
                      SOVERSION ${TBOX_ASYNC_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_ASYNC_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_util tbox_event tbox_eventx rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_ASYNC_HEADERS} DESTINATION include/tbox/async)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
