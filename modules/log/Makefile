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

PROJECT = log
LIB_NAME = log
LIB_VERSION_X = 0
LIB_VERSION_Y = 1
LIB_VERSION_Z = 0

HEAD_FILES = \
	sink.h \
	async_sink.h \
	sync_stdout_sink.h \
	async_stdout_sink.h \
	async_syslog_sink.h \
	async_file_sink.h \

CPP_SRC_FILES = \
	sink.cpp \
	sync_stdout_sink.cpp \
	async_sink.cpp \
	async_stdout_sink.cpp \
	async_syslog_sink.cpp \
	async_file_sink.cpp \

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	async_sink_test.cpp \
	async_stdout_sink_test.cpp \
	async_syslog_sink_test.cpp \
	async_file_sink_test.cpp \
	sync_stdout_sink_test.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.log"' $(CXXFLAGS)

TEST_LDFLAGS := $(LDFLAGS) -ltbox_util -ltbox_event -ltbox_base -ldl
ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
