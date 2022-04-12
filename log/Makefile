LIB_NAME = log
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	channel.h \
	async_channel.h \
	stdout_channel.h \
	syslog_channel.h \

CPP_SRC_FILES = \
	channel.cpp \
	async_channel.cpp \
	stdout_channel.cpp \
	syslog_channel.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"log"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	stdout_channel_test.cpp \
	syslog_channel_test.cpp \
	async_channel_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_base -ltbox_util
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
