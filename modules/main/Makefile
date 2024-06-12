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

PROJECT = main
LIB_NAME = main
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 4

HEAD_FILES = \
	context.h \
	main.h \
	module.h \

CPP_SRC_FILES = \
	context_imp.cpp \
	run_in_frontend.cpp \
	run_in_backend.cpp \
	error_signals.cpp \
	terminate.cpp \
	misc.cpp \
	args.cpp \
	module.cpp \
	log.cpp \
	trace.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.main"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS) \
	-ltbox_terminal \
	-ltbox_network \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_log \
	-ltbox_util \
	-ltbox_base \
	-lpthread -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
