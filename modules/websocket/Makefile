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
# Copyright (c) 2026 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

PROJECT = websocket
LIB_NAME = websocket

LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	ws_frame.h \
	server/ws_server.h \
	client/ws_client.h \

CPP_SRC_FILES = \
	ws_frame_parser.cpp \
	ws_frame_builder.cpp \
	server/ws_connection.cpp \
	server/ws_server_impl.cpp \
	client/ws_client.cpp \
	client/ws_client_impl.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.ws"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	server/ws_server_impl_test.cpp \
	ws_frame_parser_test.cpp \
	ws_frame_builder_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_crypto -ltbox_http -ltbox_network -ltbox_log -ltbox_eventx -ltbox_event -ltbox_util -ltbox_base -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
