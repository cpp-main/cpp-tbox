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

PROJECT = terminal
LIB_NAME = terminal
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	types.h \
	connection.h \
	terminal_interact.h \
	terminal_nodes.h \
	helper.h \
	session.h \
	terminal.h \
	service/telnetd.h \
	service/tcp_rpc.h \
	service/stdio.h \

CPP_SRC_FILES = \
	terminal.cpp \
	session.cpp \
	service/telnetd.cpp \
	service/tcp_rpc.cpp \
	service/stdio.cpp \
	helper.cpp \
	impl/terminal.cpp \
	impl/terminal_key_events.cpp \
	impl/terminal_commands.cpp \
	impl/terminal_nodes.cpp \
	impl/key_event_scanner.cpp \
	impl/dir_node.cpp \
	impl/func_node.cpp \
	impl/service/telnetd.cpp \
	impl/service/tcp_rpc.cpp \
	impl/service/stdio.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.terminal"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	impl/key_event_scanner.cpp \
	impl/key_event_scanner_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_event -ltbox_util -ltbox_base -ldl
ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
