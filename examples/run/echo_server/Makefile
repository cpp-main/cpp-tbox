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

PROJECT = examples/run/echo_server

LIB_NAME = echo_server
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

CPP_SRC_FILES = echo_server.cpp

CXXFLAGS := -DMODULE_ID='"$(LIB_NAME)"' $(CXXFLAGS)

ENABLE_STATIC_LIB = no
ENABLE_SHARED_LIB = yes

include $(TOP_DIR)/mk/lib_common.mk
