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

PROJECT = alarm
LIB_NAME = alarm
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	alarm.h \
	weekly_alarm.h \
	oneshot_alarm.h \
	cron_alarm.h \
	workday_alarm.h \
	workday_calendar.h \

CPP_SRC_FILES = \
	alarm.cpp \
	weekly_alarm.cpp \
	oneshot_alarm.cpp \
	cron_alarm.cpp \
	3rd-party/ccronexpr.cpp \
	workday_alarm.cpp \
	workday_calendar.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.alarm"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	workday_calendar_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base -ldl
ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
