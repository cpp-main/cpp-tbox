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

PROJECT := examples/dbus/00-loop
EXE_NAME := ${PROJECT}

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DMODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
CXXFLAGS += $(shell pkg-config --cflags dbus-1) 
LDFLAGS += $(shell pkg-config --libs dbus-1)
LDFLAGS += \
	-ltbox_dbus \
	-ltbox_event \
	-ltbox_base \
	-ldl

include $(TOP_DIR)/mk/exe_common.mk
