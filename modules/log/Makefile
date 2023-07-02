PROJECT = log
LIB_NAME = log
LIB_VERSION_X = 0
LIB_VERSION_Y = 1
LIB_VERSION_Z = 0

HEAD_FILES = \
	sink.h \
	async_sink.h \
	async_stdout_sink.h \
	async_syslog_sink.h \
	async_file_sink.h \

CPP_SRC_FILES = \
	sink.cpp \
	async_sink.cpp \
	async_stdout_sink.cpp \
	async_syslog_sink.cpp \
	async_file_sink.cpp \

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	async_sink_test.cpp \
	async_stdout_sink_test.cpp \
	async_syslog_sink_test.cpp \
	async_file_sink_test.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox.log"' $(CXXFLAGS)

TEST_LDFLAGS := $(LDFLAGS) -ltbox_util -ltbox_event -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_tbox_common.mk
