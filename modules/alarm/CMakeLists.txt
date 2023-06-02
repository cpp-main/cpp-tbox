cmake_minimum_required(VERSION 3.15)

set(TBOX_ALARM_VERSION_MAJOR 0)
set(TBOX_ALARM_VERSION_MINOR 0)
set(TBOX_ALARM_VERSION_PATCH 1)
set(TBOX_ALARM_VERSION ${TBOX_ALARM_VERSION_MAJOR}.${TBOX_ALARM_VERSION_MINOR}.${TBOX_ALARM_VERSION_PATCH})

project(tbox_alarm)

add_definitions(-DLOG_MODULE_ID="tbox.alarm")

set(TBOX_ALARM_HEADERS
    alarm.h
    weekly_alarm.h
    oneshot_alarm.h
    cron_alarm.h
    workday_alarm.h
    workday_calendar.h)

set(TBOX_ALARM_SOURCES
    alarm.cpp
    weekly_alarm.cpp
    oneshot_alarm.cpp
    cron_alarm.cpp
    3rd-party/ccronexpr.cpp
    workday_alarm.cpp
    workday_calendar.cpp)

set(TBOX_ALARM_TEST_SOURCES
    workday_calendar_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_ALARM_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_ALARM_VERSION}
                      SOVERSION ${TBOX_ALARM_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_ALARM_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base tbox_event rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_ALARM_HEADERS} DESTINATION include/tbox/alarm)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
