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

PROJECT := examples/trace/01_demo
EXE_NAME := ${PROJECT}

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DMODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_trace \
	-ltbox_util \
	-ltbox_base \
	-lpthread -ldl

include $(TOP_DIR)/mk/exe_common.mk
