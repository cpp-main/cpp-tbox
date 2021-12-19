LIB_NAME = util
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	async_data_stream.h

SRC_FILES = \
	async_data_stream.cpp

CCXXFLAGS += -DLOG_MODULE_ID='"util"' $(CXXFLAGS)

TEST_SRC_FILES = \
	async_data_stream_test.cpp

TEST_LDFLAGS := $(LDFLAGS)

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
