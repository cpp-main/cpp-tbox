cmake_minimum_required(VERSION 3.15)

set(TBOX_COROUTINE_VERSION_MAJOR 0)
set(TBOX_COROUTINE_VERSION_MINOR 0)
set(TBOX_COROUTINE_VERSION_PATCH 1)
set(TBOX_COROUTINE_VERSION ${TBOX_COROUTINE_VERSION_MAJOR}.${TBOX_COROUTINE_VERSION_MINOR}.${TBOX_COROUTINE_VERSION_PATCH})

project(tbox_coroutine)

add_definitions(-DLOG_MODULE_ID="tbox.coroutine")

set(TBOX_COROUTINE_HEADERS
    scheduler.h
    channel.hpp
    semaphore.hpp
    mutex.hpp
    broadcast.hpp
    condition.hpp)

set(TBOX_COROUTINE_SOURCES
    scheduler.cpp)

set(TBOX_COROUTINE_TEST_SOURCES
    scheduler_test.cpp
    channel_test.cpp
    semaphore_test.cpp
    mutex_test.cpp
    broadcast_test.cpp
    condition_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_COROUTINE_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_COROUTINE_VERSION}
                      SOVERSION ${TBOX_COROUTINE_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_COROUTINE_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_event rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_COROUTINE_HEADERS} DESTINATION include/tbox/coroutine)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
