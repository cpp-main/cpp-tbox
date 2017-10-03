LIB_NAME = eventx
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	thread_pool.h

SRC_FILES = \
	thread_pool.cpp

CCXXFLAGS += -I../base -DLOG_MODULE_ID='"eventx"' -Wall -Werror $(CXXFLAGS) -ggdb
TEST_LDFLAGS := $(LDFLAGS) -levent-ne -levent_core -lev

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
