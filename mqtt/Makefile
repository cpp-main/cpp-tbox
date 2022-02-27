LIB_NAME = mqtt
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = client.h

CPP_SRC_FILES = client.cpp

CXXFLAGS := -DLOG_MODULE_ID='"mqtt"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base -lmosquitto -levent_core -lev

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
