if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_event.la
else
noinst_LTLIBRARIES = libbox_event.la
endif

MAINTAINERCLEANFILES = Makefile.in

ADDITIONAL_CXXFLAGS =
VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/event

HEAD_FILES = forward.h \
			 loop.h \
			 event.h \
			 fd_event.h \
			 timer_event.h \
			 signal_event.h \
			 stat.h \
			 version.h

CPP_SRC_FILES = \
				loop.cpp \
				common_loop.cpp \
				common_loop_signal.cpp \
				common_loop_run.cpp \
				signal_event_impl.cpp \
				misc.cpp \
				stat.cpp \
				version.cpp

if ENABLE_LIBEVENT
ADDITIONAL_CXXFLAGS += -DENABLE_LIBEVENT=1
CPP_SRC_FILES += \
	engins/libevent/loop.cpp \
	engins/libevent/fd_event.cpp \
	engins/libevent/timer_event.cpp \
	engins/libevent/common.cpp
endif

if ENABLE_LIBEV
ADDITIONAL_CXXFLAGS += -DENABLE_LIBEV=1
CPP_SRC_FILES += \
	engins/libev/loop.cpp \
	engins/libev/fd_event.cpp \
	engins/libev/timer_event.cpp
endif

if ENABLE_EPOLL
ADDITIONAL_CXXFLAGS += -DENABLE_LIBEPOLL=1
CPP_SRC_FILES += \
	engins/epoll/loop.cpp \
	engins/epoll/fd_event.cpp \
	engins/epoll/timer_event.cpp 
endif

ADDITIONAL_CXXFLAGS += -DLOG_MODULE_ID='"event"'

libtbox_event_la_SOURCES = $(CPP_SRC_FILES)
libtbox_event_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_event_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
