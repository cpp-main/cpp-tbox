include config.mk

LIB_NAME = event
LIB_VERSION_X = 1
LIB_VERSION_Y = 1
LIB_VERSION_Z = 0

HEAD_FILES = \
	forward.h loop.h event.h \
	fd_event.h timer_event.h signal_event.h \
	stat.h version.h

CPP_SRC_FILES = \
	loop.cpp \
	common_loop.cpp \
	common_loop_signal.cpp \
	common_loop_run.cpp \
	signal_event_impl.cpp \
	misc.cpp \
	stat.cpp \
	version.cpp \

ifeq ($(ENABLE_LIBEVENT), yes)
CPP_SRC_FILES += \
	engins/libevent/loop.cpp \
	engins/libevent/fd_event.cpp \
	engins/libevent/timer_event.cpp \
	engins/libevent/common.cpp
CXXFLAGS += -DENABLE_LIBEVENT=1
endif

ifeq ($(ENABLE_LIBEV), yes)
CPP_SRC_FILES += \
	engins/libev/loop.cpp \
	engins/libev/fd_event.cpp \
	engins/libev/timer_event.cpp
CXXFLAGS += -DENABLE_LIBEV=1
endif

ifeq ($(ENABLE_EPOLL), yes)
CPP_SRC_FILES += \
	engins/epoll/loop.cpp \
	engins/epoll/fd_event.cpp \
	engins/epoll/timer_event.cpp
CXXFLAGS += -DENABLE_EPOLL=1
endif

TEST_CPP_SRC_FILES = \
	common_loop_test.cpp \
	fd_event_test.cpp \
	timer_event_test.cpp \
	signal_event_test.cpp \
	stat_test.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox_event"' $(CXXFLAGS)

ifeq ($(ENABLE_STAT), yes)
CXXFLAGS += -DENABLE_STAT=1
endif

TEST_LDFLAGS := $(LDFLAGS) -ltbox_base

ifeq ($(ENABLE_LIBEVENT), yes)
	TEST_LDFLAGS += -levent_core
endif

ifeq ($(ENABLE_LIBEV), yes)
	TEST_LDFLAGS += -lev
endif

ENABLE_SHARED_LIB = no

include ../../tools/lib_common.mk
