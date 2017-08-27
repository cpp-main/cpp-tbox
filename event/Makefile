LIB_NAME = event
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	forward.h loop.h item.h timespan.h \
	fd_item.h timer_item.h signal_item.h \
	version.h

SRC_FILES = \
	loop.cpp common_loop.cpp version.cpp \
	engins/libevent/loop.cpp \
	engins/libevent/fd_item.cpp \
	engins/libevent/timer_item.cpp \
	engins/libevent/signal_item.cpp

CXXFLAGS += -DLOG_MODULE_ID='"event"'

TEST_LDFLAGS := $(LDFLAGS) -levent_core

include ../tools/lib_common.mk
