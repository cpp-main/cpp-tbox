cmake_minimum_required(VERSION 3.15)

set(TBOX_CRYPTO_VERSION_MAJOR 0)
set(TBOX_CRYPTO_VERSION_MINOR 0)
set(TBOX_CRYPTO_VERSION_PATCH 1)
set(TBOX_CRYPTO_VERSION ${TBOX_CRYPTO_VERSION_MAJOR}.${TBOX_CRYPTO_VERSION_MINOR}.${TBOX_CRYPTO_VERSION_PATCH})

project(tbox_crypto)

add_definitions(-DLOG_MODULE_ID="tbox.crypto")

set(TBOX_CRYPTO_HEADERS
    md5.h
    aes.h)

set(TBOX_CRYPTO_SOURCES
    md5.cpp
    aes.cpp)

set(TBOX_CRYPTO_TEST_SOURCES
    md5_test.cpp
    aes_test.cpp)

add_library(${PROJECT_NAME} ${TBOX_BUILD_LIB_TYPE} ${TBOX_CRYPTO_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${TBOX_CRYPTO_VERSION}
                      SOVERSION ${TBOX_CRYPTO_VERSION_MAJOR})

if(${TBOX_ENABLE_TEST})
    add_executable(${PROJECT_NAME}_test ${TBOX_CRYPTO_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_test gmock_main gmock gtest pthread ${PROJECT_NAME} tbox_util tbox_base rt dl)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()

install(FILES ${TBOX_CRYPTO_HEADERS} DESTINATION include/tbox/crypto)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
