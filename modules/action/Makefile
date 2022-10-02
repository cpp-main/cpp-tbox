LIB_NAME = action
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	action.h \
	executor.h \

CPP_SRC_FILES = \
	action.cpp \
	executor.cpp \
	event_publisher_impl.cpp \
	actions/sleep_for_action.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"action"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	actions/sleep_for_action_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_action -ltbox_event -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_common.mk
