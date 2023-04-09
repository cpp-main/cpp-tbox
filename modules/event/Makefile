LIB_NAME = event
LIB_VERSION_X = 1
LIB_VERSION_Y = 1
LIB_VERSION_Z = 1

HEAD_FILES = \
	forward.h \
	loop.h \
	event.h \
	fd_event.h \
	timer_event.h \
	signal_event.h \
	stat.h

CPP_SRC_FILES = \
	loop.cpp \
	common_loop.cpp \
	common_loop_timer.cpp \
	common_loop_signal.cpp \
	common_loop_run.cpp \
	timer_event_impl.cpp \
	signal_event_impl.cpp \
	misc.cpp \
	stat.cpp \
	engins/epoll/loop.cpp \
	engins/epoll/fd_event.cpp \

TEST_CPP_SRC_FILES = \
	common_loop_test.cpp \
	fd_event_test.cpp \
	timer_event_test.cpp \
	signal_event_test.cpp \
	stat_test.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox.event"' $(CXXFLAGS)

TEST_LDFLAGS := $(LDFLAGS) -ltbox_base

ENABLE_SHARED_LIB = no

include ../../tools/lib_tbox_common.mk
