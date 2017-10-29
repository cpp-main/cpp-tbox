LIB_NAME = network
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	buffer.h

SRC_FILES = \
	buffer.cpp

CCXXFLAGS += -I../base -DLOG_MODULE_ID='"network"' -Wall -Werror $(CXXFLAGS) -ggdb
TEST_LDFLAGS := $(LDFLAGS) -ltbox_event-ne -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
