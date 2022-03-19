LIB_NAME = terminal
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	state_machine.h \

CPP_SRC_FILES = \
	state_machine.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"sm"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	impl/key_event_scanner_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_event -ltbox_util -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
