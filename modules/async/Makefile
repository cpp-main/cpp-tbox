LIB_NAME = async
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	async.h \

CPP_SRC_FILES = \
	async.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox.alarm"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_eventx -ltbox_event -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_tbox_common.mk
