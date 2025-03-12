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

PROJECT = base
LIB_NAME = base
LIB_NAME_EXT =

LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HAVE_EXECINFO_H ?= yes

HEAD_FILES = \
	version.h \
	log.h \
	log_impl.h \
	log_output.h \
	defines.h \
	scope_exit.hpp \
	cabinet.hpp \
	cabinet_token.h \
	memblock.h \
	json_fwd.h \
	json.hpp \
	assert.h \
	lifetime_tag.hpp \
	backtrace.h \
	catch_throw.h \
	object_pool.hpp \
	func_types.h \
	recorder.h \
	wrapped_recorder.h \

CPP_SRC_FILES = \
	version.cpp \
	log_impl.cpp \
	log_output.cpp \
	backtrace.cpp \
	catch_throw.cpp \
	recorder.cpp \

pre_build:
	touch version.cpp

CXXFLAGS += -DMODULE_ID='"tbox.base"'

ifeq ($(HAVE_EXECINFO_H),yes)
CXXFLAGS += -DHAVE_EXECINFO_H=1
else
CXXFLAGS += -DHAVE_EXECINFO_H=0
endif

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	log_output_test.cpp \
	scope_exit_test.cpp \
	cabinet_token_test.cpp \
	cabinet_test.cpp \
	json_test.cpp \
	lifetime_tag_test.cpp \
	backtrace_test.cpp \
	catch_throw_test.cpp \
	object_pool_test.cpp \
	recorder_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
