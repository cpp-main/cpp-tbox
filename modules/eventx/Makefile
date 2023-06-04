LIB_NAME = eventx
LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	thread_pool.h \
	timer_pool.h \
	timeout_monitor.hpp \
	timeout_monitor_impl.hpp \
	request_pool.hpp \
	loop_wdog.h \
	work_thread.h \
	loop_thread.h \

CPP_SRC_FILES = \
	thread_pool.cpp \
	timer_pool.cpp \
	loop_wdog.cpp \
	work_thread.cpp \
	loop_thread.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox.eventx"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	thread_pool_test.cpp \
	timer_pool_test.cpp \
	timeout_monitor_test.cpp \
	request_pool_test.cpp \
	loop_wdog_test.cpp \
	work_thread_test.cpp \
	loop_thread_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base -ldl
ENABLE_SHARED_LIB = no

include ../../tools/lib_tbox_common.mk
