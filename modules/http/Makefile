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

PROJECT = http
LIB_NAME = http
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	common.h \
	request.h \
	respond.h \
	url.h \
	server/types.h \
	server/server.h \
	server/context.h \
	server/middleware.h \
	server/middlewares/router_middleware.h \
	server/middlewares/form_data.h \
	server/middlewares/form_data_middleware.h \
	server/middlewares/file_downloader_middleware.h \
	client/client.h \

CPP_SRC_FILES = \
	common.cpp \
	request.cpp \
	respond.cpp \
	url.cpp \
	server/request_parser.cpp \
	server/server.cpp \
	server/server_imp.cpp \
	server/context.cpp \
	server/middlewares/router_middleware.cpp \
	server/middlewares/form_data.cpp \
	server/middlewares/form_data_middleware.cpp \
	server/middlewares/file_downloader_middleware.cpp \
	client/client.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.http"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	common_test.cpp \
	respond_test.cpp \
	request_test.cpp \
	url_test.cpp \
	server/request_parser_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_log -ltbox_eventx -ltbox_event -ltbox_util -ltbox_base -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
