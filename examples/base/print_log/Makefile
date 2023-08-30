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

PROJECT := example/base/print_log
EXE_NAME := $(PROJECT)

CPP_SRC_FILES := main.cpp
C_SRC_FILES := other.c

CXXFLAGS += -DLOG_MODULE_ID='"demo"'
CFLAGS += -DLOG_MODULE_ID='"demo"'

LDFLAGS += -ltbox_base

include ${TOP_DIR}/tools/exe_common.mk
