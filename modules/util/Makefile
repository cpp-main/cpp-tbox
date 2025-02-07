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

PROJECT = util
LIB_NAME = util
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	pid_file.h \
	string.h \
	fs.h \
	argument_parser.h \
	split_cmdline.h \
	serializer.h \
	async_pipe.h \
	timestamp.h \
	json.h \
	base64.h \
	checksum.h \
	crc.h \
	json_deep_loader.h \
	execute_cmd.h \
	buffer.h \
	fd.h \
	scalable_integer.h \
	variables.h \

CPP_SRC_FILES = \
	pid_file.cpp \
	string.cpp \
	fs.cpp \
	argument_parser.cpp \
	split_cmdline.cpp \
	serializer.cpp \
	async_pipe.cpp \
	timestamp.cpp \
	json.cpp \
	base64.cpp \
	checksum.cpp \
	crc.cpp \
	json_deep_loader.cpp \
	execute_cmd.cpp \
	buffer.cpp \
	fd.cpp \
	scalable_integer.cpp \
	variables.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.util"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	pid_file_test.cpp \
	string_test.cpp \
	fs_test.cpp \
	argument_parser_test.cpp \
	split_cmdline_test.cpp \
	serializer_test.cpp \
	async_pipe_test.cpp \
	json_test.cpp \
	base64_test.cpp \
	checksum_test.cpp \
	crc_test.cpp \
	execute_cmd_test.cpp \
	timestamp_test.cpp \
	buffer_test.cpp \
	fd_test.cpp \
	scalable_integer_test.cpp \
	variables_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_base -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
