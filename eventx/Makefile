LIB_NAME = eventx
LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	thread_pool.h \
	timer_pool.h \

CPP_SRC_FILES = \
	thread_pool.cpp \
	timer_pool.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"eventx"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	thread_pool_test.cpp \
	timer_pool_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event-ne -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
