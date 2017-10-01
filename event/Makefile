include config.mk

LIB_NAME = event
LIB_NAME_EXT = -
LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 0

HEAD_FILES = \
	forward.h loop.h item.h timespan.h \
	fd_item.h timer_item.h signal_item.h \
	stat.h version.h

SRC_FILES = \
	loop.cpp common_loop.cpp \
	version.cpp

ifeq ($(ENABLE_LIBEVENT), yes)
SRC_FILES += \
	engins/libevent/loop.cpp \
	engins/libevent/fd_item.cpp \
	engins/libevent/timer_item.cpp \
	engins/libevent/signal_item.cpp
CCXXFLAGS += -DENABLE_LIBEVENT=1
LIB_NAME_EXT := $(LIB_NAME_EXT)n
endif

ifeq ($(ENABLE_LIBEV), yes)
SRC_FILES += \
	engins/libev/loop.cpp \
	engins/libev/fd_item.cpp \
	engins/libev/signal_item.cpp \
	engins/libev/timer_item.cpp
CCXXFLAGS += -DENABLE_LIBEV=1
LIB_NAME_EXT := $(LIB_NAME_EXT)e
endif

CCXXFLAGS += -DLOG_MODULE_ID='"tbox_event"' $(CXXFLAGS)

ifeq ($(ENABLE_STAT), yes)
CCXXFLAGS += -DENABLE_STAT=1
endif

TEST_LDFLAGS := $(LDFLAGS)

ifeq ($(ENABLE_LIBEVENT), yes)
	TEST_LDFLAGS += -levent_core
endif

ifeq ($(ENABLE_LIBEV), yes)
	TEST_LDFLAGS += -lev
endif

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
