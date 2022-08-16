LIB_NAME = eventx
LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	thread_pool.h \
	timer_pool.h \
	timeout_monitor.h \
	request_pool.hpp \

CPP_SRC_FILES = \
	thread_pool.cpp \
	timer_pool.cpp \
	timeout_monitor.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"eventx"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	thread_pool_test.cpp \
	timer_pool_test.cpp \
	timeout_monitor_test.cpp \
	request_pool_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include $(TOP_DIR)/tools/lib_common.mk
