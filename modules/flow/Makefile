LIB_NAME = flow
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	state_machine.h \
	action.h \
	event.h \
	action_executor.h \
	event_subscriber.h \
	event_publisher.h \
	event_publisher_impl.h \
	actions/function_action.h \
	actions/sleep_action.h \
	actions/sequence_action.h \
	actions/parallel_action.h \
	actions/invert_action.h \
	actions/if_else_action.h \
	actions/loop_action.h \
	actions/loop_if_action.h \
	actions/event_action.h \
	actions/repeat_action.h \

CPP_SRC_FILES = \
	state_machine.cpp \
	action.cpp \
	action_executor.cpp \
	event_publisher_impl.cpp \
	actions/function_action.cpp \
	actions/sleep_action.cpp \
	actions/sequence_action.cpp \
	actions/parallel_action.cpp \
	actions/invert_action.cpp \
	actions/if_else_action.cpp \
	actions/loop_action.cpp \
	actions/loop_if_action.cpp \
	actions/event_action.cpp \
	actions/repeat_action.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"flow"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	state_machine_test.cpp \
	action_test.cpp \
	event_publisher_impl_test.cpp \
	action_executor_test.cpp \
	actions/sleep_action_test.cpp \
	actions/function_action_test.cpp \
	actions/sequence_action_test.cpp \
	actions/parallel_action_test.cpp \
	actions/invert_action_test.cpp \
	actions/if_else_action_test.cpp \
	actions/loop_action_test.cpp \
	actions/loop_if_action_test.cpp \
	actions/repeat_action_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_flow -ltbox_event -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_common.mk
