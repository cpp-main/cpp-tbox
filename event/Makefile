include config.mk

LIB_NAME = event
LIB_NAME_EXT = -
LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 0

HEAD_FILES = \
	forward.h loop.h event.h \
	fd_event.h timer_event.h signal_event.h \
	stat.h version.h

CPP_SRC_FILES = \
	loop.cpp common_loop.cpp \
	version.cpp

ifeq ($(ENABLE_LIBEVENT), yes)
CPP_SRC_FILES += \
	engins/libevent/loop.cpp \
	engins/libevent/fd_event.cpp \
	engins/libevent/timer_event.cpp \
	engins/libevent/signal_event.cpp \
	engins/libevent/common.cpp
CXXFLAGS += -DENABLE_LIBEVENT=1
LIB_NAME_EXT := $(LIB_NAME_EXT)n
endif

ifeq ($(ENABLE_LIBEV), yes)
CPP_SRC_FILES += \
	engins/libev/loop.cpp \
	engins/libev/fd_event.cpp \
	engins/libev/signal_event.cpp \
	engins/libev/timer_event.cpp
CXXFLAGS += -DENABLE_LIBEV=1
LIB_NAME_EXT := $(LIB_NAME_EXT)e
endif

CXXFLAGS := -DLOG_MODULE_ID='"tbox_event"' $(CXXFLAGS)

ifeq ($(ENABLE_STAT), yes)
CCXXFLAGS += -DENABLE_STAT=1
endif

TEST_LDFLAGS := $(LDFLAGS) -ltbox_base

ifeq ($(ENABLE_LIBEVENT), yes)
	TEST_LDFLAGS += -levent_core
endif

ifeq ($(ENABLE_LIBEV), yes)
	TEST_LDFLAGS += -lev
endif

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
