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

PROJECT := examples/main/sample
EXE_NAME := ${PROJECT}

CPP_SRC_FILES := app_main.cpp build_time.cpp
TEST_CPP_SRC_FILES :=
TEST_LDFLAGS :=

CONF_FILES := default.conf

include app1/app.mk
include app2/app.mk

CXXFLAGS := -DMODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_main \
	-ltbox_coroutine \
	-ltbox_trace \
	-ltbox_terminal \
	-ltbox_network \
	-ltbox_eventx \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_log \
	-ltbox_util \
	-ltbox_base \
	-lpthread \
	-ldl \
	-rdynamic

TEST_LDFLAGS += $(LDFLAGS)

#pre_build :
#	touch build_time.cpp

include $(TOP_DIR)/mk/exe_common.mk
