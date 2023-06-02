cmake_minimum_required(VERSION 3.15)

set(TBOX_MAIN_VERSION_MAJOR 1)
set(TBOX_MAIN_VERSION_MINOR 1)
set(TBOX_MAIN_VERSION_PATCH 2)
set(TBOX_MAIN_VERSION ${TBOX_MAIN_VERSION_MAJOR}.${TBOX_MAIN_VERSION_MINOR}.${TBOX_MAIN_VERSION_PATCH})

project(tbox_main)

add_definitions(-DLOG_MODULE_ID="tbox.main")

set(TBOX_MAIN_HEADERS
    context.h
    main.h
    module.h)

set(TBOX_MAIN_SOURCES
    context_imp.cpp
    run_in_frontend.cpp
    run_in_backend.cpp
    error_signals.cpp
    terminate.cpp
    misc.cpp
    args.cpp
    module.cpp
    log.cpp)

#set(TBOX_MAIN_TEST_SOURCES )

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_MAIN_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_MAIN_VERSION}
                      SOVERSION ${TBOX_MAIN_VERSION_MAJOR})

#if(${TBOX_ENABLE_TEST})
    #add_executable(${PROJECT_NAME}_test ${TBOX_MAIN_TEST_SOURCES})
    #target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_base rt dl)
    #add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
#endif()

install(FILES ${TBOX_MAIN_HEADERS} DESTINATION include/tbox/main)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
