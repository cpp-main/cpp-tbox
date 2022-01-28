LIB_NAME = telnet
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	telnetd.h \

CPP_SRC_FILES = \
	telnetd.cpp \
	impl/impl.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"telnetd"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	telnetd_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_event-ne -ltbox_util -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
