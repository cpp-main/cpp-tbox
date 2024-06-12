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
# Copyright (c) 2023 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

PROJECT = jsonrpc
LIB_NAME = jsonrpc
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	proto.h \
	protos/raw_stream_proto.h \
	protos/header_stream_proto.h \
	protos/packet_proto.h \
	rpc.h \

CPP_SRC_FILES = \
	proto.cpp \
	protos/raw_stream_proto.cpp \
	protos/header_stream_proto.cpp \
	protos/packet_proto.cpp \
	rpc.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.jsonrpc"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	protos/raw_stream_proto_test.cpp \
	protos/header_stream_proto_test.cpp \
	protos/packet_proto_test.cpp \
	rpc_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_util -ltbox_base -ldl
ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
