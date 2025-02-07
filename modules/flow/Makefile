#
#     .============.
#    //  M A K E  / \
#   //  C++ DEV  /   \
#  //  E A S Y  /  \/ \
# ++ ----------.  \/\  .
#  \\     \     \ /\  /
#   \\     \     \   /
#    \\     \     \ /
#     -============'
#
# Copyright (c) 2018 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

PROJECT = flow
LIB_NAME = flow
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 3

HEAD_FILES = \
	state_machine.h \
	action.h \
	action_reason.h \
	event.h \
	action_executor.h \
	event_subscriber.h \
	event_publisher.h \
	event_publisher_impl.h \
	actions/assemble_action.h \
	actions/function_action.h \
	actions/sleep_action.h \
	actions/sequence_action.h \
	actions/parallel_action.h \
	actions/if_else_action.h \
	actions/if_then_action.h \
	actions/loop_action.h \
	actions/loop_if_action.h \
	actions/event_action.h \
	actions/repeat_action.h \
	actions/composite_action.h \
	actions/wrapper_action.h \
	actions/succ_fail_action.h \
	actions/dummy_action.h \
	actions/switch_action.h \
	to_graphviz.h \

CPP_SRC_FILES = \
	state_machine.cpp \
	action.cpp \
	action_executor.cpp \
	event_publisher_impl.cpp \
	actions/assemble_action.cpp \
	actions/function_action.cpp \
	actions/sleep_action.cpp \
	actions/sequence_action.cpp \
	actions/parallel_action.cpp \
	actions/if_else_action.cpp \
	actions/if_then_action.cpp \
	actions/loop_action.cpp \
	actions/loop_if_action.cpp \
	actions/event_action.cpp \
	actions/repeat_action.cpp \
	actions/composite_action.cpp \
	actions/wrapper_action.cpp \
	actions/dummy_action.cpp \
	actions/switch_action.cpp \
	to_graphviz.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.flow"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	state_machine_test.cpp \
	action_test.cpp \
	event_publisher_impl_test.cpp \
	action_executor_test.cpp \
	actions/assemble_action_test.cpp \
	actions/sleep_action_test.cpp \
	actions/function_action_test.cpp \
	actions/sequence_action_test.cpp \
	actions/parallel_action_test.cpp \
	actions/if_else_action_test.cpp \
	actions/if_then_action_test.cpp \
	actions/loop_action_test.cpp \
	actions/loop_if_action_test.cpp \
	actions/repeat_action_test.cpp \
	actions/composite_action_test.cpp \
	actions/wrapper_action_test.cpp \
	actions/succ_fail_action_test.cpp \
	actions/switch_action_test.cpp \
	to_graphviz_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_flow -ltbox_eventx -ltbox_event -ltbox_util -ltbox_base -ldl
ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
