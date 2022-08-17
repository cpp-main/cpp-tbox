LIB_NAME = util
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	pid_file.h \
	thread_wdog.h \
	string.h \
	fs.h \
	argument_parser.h \
	split_cmdline.h \
	serializer.h \
	time_counter.h \
	state_machine.h \
	async_pipe.h \
	backtrace.h \

CPP_SRC_FILES = \
	pid_file.cpp \
	thread_wdog.cpp \
	string.cpp \
	fs.cpp \
	argument_parser.cpp \
	split_cmdline.cpp \
	serializer.cpp \
	time_counter.cpp \
	state_machine.cpp \
	async_pipe.cpp \
	backtrace.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"util"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	thread_wdog_test.cpp \
	pid_file_test.cpp \
	string_test.cpp \
	fs_test.cpp \
	argument_parser_test.cpp \
	split_cmdline_test.cpp \
	serializer_test.cpp \
	time_counter_test.cpp \
	state_machine_test.cpp \
	async_pipe_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_base -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/tools/lib_common.mk
