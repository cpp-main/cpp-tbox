LIB_NAME = util
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	async_data_stream.h \
	pid_file.h \
	thread_wdog.h \
	string.h \
	fs.h \
	argument_parser.h \

SRC_FILES = \
	async_data_stream.cpp \
	pid_file.cpp \
	thread_wdog.cpp \
	string.cpp \
	fs.cpp \
	argument_parser.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"util"' $(CXXFLAGS)

TEST_SRC_FILES = \
	async_data_stream_test.cpp \
	thread_wdog_test.cpp \
	pid_file_test.cpp \
	string_test.cpp \
	fs_test.cpp \
	argument_parser_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_base

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
