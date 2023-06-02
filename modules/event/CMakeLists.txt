cmake_minimum_required(VERSION 3.15)

set(TBOX_EVENT_VERSION_MAJOR 1)
set(TBOX_EVENT_VERSION_MINOR 1)
set(TBOX_EVENT_VERSION_PATCH 2)
set(TBOX_EVENT_VERSION ${TBOX_EVENT_VERSION_MAJOR}.${TBOX_EVENT_VERSION_MINOR}.${TBOX_EVENT_VERSION_PATCH})

project(tbox_event)

add_definitions(-DLOG_MODULE_ID="tbox.event")

set(TBOX_EVENT_HEADERS
            forward.h
            loop.h
            event.h
            fd_event.h
            timer_event.h
            signal_event.h
            stat.h)

set(TBOX_EVENT_SOURCES
            loop.cpp
            common_loop.cpp
            common_loop_timer.cpp
            common_loop_signal.cpp
            common_loop_run.cpp
            timer_event_impl.cpp
            signal_event_impl.cpp
            misc.cpp
            stat.cpp
            engins/epoll/loop.cpp
            engins/epoll/fd_event.cpp)

set(TBOX_EVENT_TEST_SOURCES
            common_loop_test.cpp
            fd_event_test.cpp
            timer_event_test.cpp
            signal_event_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_EVENT_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_EVENT_VERSION}
                      SOVERSION ${TBOX_EVENT_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_EVENT_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_EVENT_HEADERS} DESTINATION include/tbox/event)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
