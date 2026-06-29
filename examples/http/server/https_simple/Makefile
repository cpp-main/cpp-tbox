#
#     .============.
#    //  M A K E  / \
#   //  C++ DEV  /   \
#   //  E A S Y  /  \/ \
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
# in the LICENSE file in the root of the source tree.
#

PROJECT := examples/http/server/https_simple
EXE_NAME := ${PROJECT}

CPP_SRC_FILES := https_simple.cpp

CONF_FILES := server.crt server.key

CXXFLAGS := -DMODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_http \
	-ltbox_network \
	-Wl,--whole-archive -ltbox_network_tls -Wl,--no-whole-archive \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_log \
	-ltbox_util \
	-ltbox_base \
	-lssl -lcrypto \
	-lpthread -ldl

include $(TOP_DIR)/mk/exe_common.mk
