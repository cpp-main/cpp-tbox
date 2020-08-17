LIB_NAME = mqtt
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = client.h

SRC_FILES = client.cpp

CCXXFLAGS += -DLOG_MODULE_ID='"mqtt"' $(CXXFLAGS) -ggdb

TEST_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event-ne -ltbox_base -levent_core -lev

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
