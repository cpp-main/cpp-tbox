LIB_NAME = eventx
LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	thread_pool.h \
	timer_pool.h \
	timeout_monitor.h \
	request_pool.hpp \
	loop_wdog.h \
	work_thread.h \

CPP_SRC_FILES = \
	thread_pool.cpp \
	timer_pool.cpp \
	timeout_monitor.cpp \
	loop_wdog.cpp \
	work_thread.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"eventx"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	thread_pool_test.cpp \
	timer_pool_test.cpp \
	timeout_monitor_test.cpp \
	request_pool_test.cpp \
	loop_wdog_test.cpp \
	work_thread_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_common.mk
