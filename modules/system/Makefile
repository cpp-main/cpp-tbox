LIB_NAME = system
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = system.h

CPP_SRC_FILES = system.cpp

CXXFLAGS := -DLOG_MODULE_ID='"tbox.system"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = system_test.cpp

TEST_LDFLAGS := $(LDFLAGS) -ltbox_eventx -ltbox_event -ltbox_util -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_tbox_common.mk
