cmake_minimum_required(VERSION 3.15)

set(TBOX_EVENTX_VERSION_MAJOR 1)
set(TBOX_EVENTX_VERSION_MINOR 0)
set(TBOX_EVENTX_VERSION_PATCH 2)
set(TBOX_EVENTX_VERSION ${TBOX_EVENTX_VERSION_MAJOR}.${TBOX_EVENTX_VERSION_MINOR}.${TBOX_EVENTX_VERSION_PATCH})

project(tbox_eventx)

add_definitions(-DLOG_MODULE_ID="tbox.eventx")

set(TBOX_EVENTX_HEADERS
    thread_pool.h
    timer_pool.h
    timeout_monitor.hpp
    timeout_monitor_impl.hpp
    request_pool.hpp
    loop_wdog.h
    work_thread.h
    loop_thread.h
    timre_fd.h)

set(TBOX_EVENTX_SOURCES
    thread_pool.cpp
    timer_pool.cpp
    loop_wdog.cpp
    work_thread.cpp
    loop_thread.cpp
    timer_fd.cpp)

set(TBOX_EVENTX_TEST_SOURCES
    thread_pool_test.cpp
    timer_pool_test.cpp
    timeout_monitor_test.cpp
    request_pool_test.cpp
    loop_wdog_test.cpp
    work_thread_test.cpp
    loop_thread_test.cpp
    timer_fd_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_EVENTX_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_EVENTX_VERSION}
                      SOVERSION ${TBOX_EVENTX_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_EVENTX_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_event rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_EVENTX_HEADERS} DESTINATION include/tbox/eventx)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
