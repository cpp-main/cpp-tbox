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

PROJECT = eventx
LIB_NAME = eventx
LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	thread_pool.h \
	timer_pool.h \
	timeout_monitor.hpp \
	timeout_monitor_impl.hpp \
	request_pool.hpp \
	loop_wdog.h \
	work_thread.h \
	loop_thread.h \
	timer_fd.h \
	async.h \

CPP_SRC_FILES = \
	thread_pool.cpp \
	timer_pool.cpp \
	loop_wdog.cpp \
	work_thread.cpp \
	loop_thread.cpp \
	timer_fd.cpp \
	async.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.eventx"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	thread_pool_test.cpp \
	timer_pool_test.cpp \
	timeout_monitor_test.cpp \
	request_pool_test.cpp \
	loop_wdog_test.cpp \
	work_thread_test.cpp \
	loop_thread_test.cpp \
	timer_fd_test.cpp \
	async_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_util -ltbox_base -ldl
ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
