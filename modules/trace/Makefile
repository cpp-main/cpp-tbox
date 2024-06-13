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
# Copyright (c) 2024 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

PROJECT = trace
LIB_NAME = trace
LIB_VERSION_X = 0
LIB_VERSION_Y = 1
LIB_VERSION_Z = 0

HEAD_FILES = sink.h

CPP_SRC_FILES = sink.cpp

CXXFLAGS := -DMODULE_ID='"tbox.trace"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	sink_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_util -ltbox_base -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
