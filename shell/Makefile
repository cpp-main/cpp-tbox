LIB_NAME = shell
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	types.h \
	interface.h \
	telnetd.h \
	shell.h \

CPP_SRC_FILES = \
	telnetd.cpp \
	shell.cpp \
	impl/shell.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"shell"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	telnetd_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_event-ne -ltbox_util -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
