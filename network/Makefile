LIB_NAME = network
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	buffer.h \
	fd.h \
	buffered_fd.h \
	byte_stream.h \
	stdio_stream.h \

SRC_FILES = \
	buffer.cpp \
	fd.cpp \
	buffered_fd.cpp \
	stdio_stream.cpp \

CCXXFLAGS += -I../base -DLOG_MODULE_ID='"network"' $(CXXFLAGS) -ggdb

TEST_SRC_FILES = \
	fd_test.cpp \
	buffer_test.cpp \
	buffered_fd_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event-ne -ltbox_base -levent_core -lev

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
