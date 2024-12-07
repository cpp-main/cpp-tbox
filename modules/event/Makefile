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

PROJECT = event
LIB_NAME = event
LIB_VERSION_X = 1
LIB_VERSION_Y = 2
LIB_VERSION_Z = 0

HAVE_EPOLL ?= yes

CXXFLAGS += -DMODULE_ID='"tbox.event"'

HEAD_FILES = \
	forward.h \
	loop.h \
	event.h \
	fd_event.h \
	timer_event.h \
	signal_event.h \
	stat.h

CPP_SRC_FILES = \
	loop.cpp \
	common_loop.cpp \
	common_loop_timer.cpp \
	common_loop_signal.cpp \
	common_loop_run.cpp \
	timer_event_impl.cpp \
	signal_event_impl.cpp \
	misc.cpp \
	stat.cpp \
	engines/select/loop.cpp \
	engines/select/fd_event.cpp \

ifeq ($(HAVE_EPOLL),yes)
CXXFLAGS += -DHAVE_EPOLL=1

CPP_SRC_FILES += \
	engines/epoll/loop.cpp \
	engines/epoll/fd_event.cpp
endif

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	common_loop_test.cpp \
	fd_event_test.cpp \
	timer_event_test.cpp \
	signal_event_test.cpp \


TEST_LDFLAGS := $(LDFLAGS) -ltbox_base -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
