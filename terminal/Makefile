LIB_NAME = terminal
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	types.h \
	interface.h \
	telnetd.h \
	terminal.h \

CPP_SRC_FILES = \
	telnetd.cpp \
	terminal.cpp \
	impl/terminal.cpp \
	impl/key_event_scanner.cpp \
	impl/dir_node.cpp \
	impl/func_node.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"terminal"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	impl/key_event_scanner_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_event-ne -ltbox_util -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
