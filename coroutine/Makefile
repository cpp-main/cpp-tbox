LIB_NAME = coroutine
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	scheduler.h \
	channel.hpp \
	semaphore.hpp \
	mutex.hpp \
	broadcast.hpp \
	condition.hpp \

SRC_FILES = \
	scheduler.cpp

CCXXFLAGS += -DLOG_MODULE_ID='"coroutine"' $(CXXFLAGS)

TEST_SRC_FILES = \
	scheduler_test.cpp \
	channel_test.cpp \
	semaphore_test.cpp \
	mutex_test.cpp \
	broadcast_test.cpp \
	condition_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event-ne -ltbox_base -levent_core -lev

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
