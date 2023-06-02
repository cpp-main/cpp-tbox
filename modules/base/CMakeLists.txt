cmake_minimum_required(VERSION 3.15)

set(TBOX_BASE_VERSION_MAJOR 1)
set(TBOX_BASE_VERSION_MINOR 0)
set(TBOX_BASE_VERSION_PATCH 1)
set(TBOX_BASE_VERSION ${TBOX_BASE_VERSION_MAJOR}.${TBOX_BASE_VERSION_MINOR}.${TBOX_BASE_VERSION_PATCH})

project(tbox_base)

add_definitions(-DLOG_MODULE_ID="tbox.base")

set(TBOX_BASE_HEADERS
    version.h
    log.h
    log_imp.h
    log_output.h
    defines.h
    scope_exit.hpp
    cabinet.hpp
    cabinet_token.h
    memblock.h
    json_fwd.h
    json.hpp
    assert.h
    lifetime_tag.hpp
    backtrace.h
    catch_throw.h
    object_pool.hpp)

set(TBOX_BASE_SOURCES
    version.cpp
    log_imp.cpp
    log_output.cpp
    backtrace.cpp
    catch_throw.cpp)

set(TBOX_BASE_TEST_SOURCES
    log_output_test.cpp
    scope_exit_test.cpp
    cabinet_token_test.cpp
    cabinet_test.cpp
    json_test.cpp
    lifetime_tag_test.cpp
    backtrace_test.cpp
    catch_throw_test.cpp
    object_pool_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_BASE_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_BASE_VERSION}
                      SOVERSION ${TBOX_BASE_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_BASE_TEST_SOURCES})

    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_BASE_HEADERS} DESTINATION include/tbox/base)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
