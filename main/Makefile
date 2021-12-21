LIB_NAME = main
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	thread_wdog.h \
	pid_file.h \

SRC_FILES = \
	main.cpp \
	thread_wdog.cpp \
	pid_file.cpp \

CCXXFLAGS += -DLOG_MODULE_ID='"main"' $(CXXFLAGS)

TEST_SRC_FILES =

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event-ne -ltbox_base -levent_core -lev

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
