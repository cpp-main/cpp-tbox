LIB_NAME = log
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	channel.h \
	stdout_channel.h \

CPP_SRC_FILES = \
	channel.cpp \
	stdout_channel.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"log"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS)
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
