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

PROJECT = coroutine
LIB_NAME = coroutine
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	scheduler.h \
	channel.hpp \
	semaphore.hpp \
	mutex.hpp \
	broadcast.hpp \
	condition.hpp \

CPP_SRC_FILES = \
	scheduler.cpp

CXXFLAGS := -DMODULE_ID='"tbox.coroutine"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	scheduler_test.cpp \
	channel_test.cpp \
	semaphore_test.cpp \
	mutex_test.cpp \
	broadcast_test.cpp \
	condition_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
