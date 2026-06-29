#
#     .============.
#    //  M A K E  / \
#   //  C++ DEV  /   \
#  //  E A S E  /  \/ \
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

PROJECT = network_tls
LIB_NAME = network_tls
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	buffered_ssl_fd.h \
	tcp_tls_factory.h \
	tcp_tls_connection.h \
	tcp_tls_connector.h \
	tcp_tls_acceptor.h \

CPP_SRC_FILES = \
	tls_factory_entry.cpp \
	buffered_ssl_fd.cpp \
	tcp_tls_factory.cpp \
	tcp_tls_connection.cpp \
	tcp_tls_connector.cpp \
	tcp_tls_acceptor.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.network_tls"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \

TEST_LDFLAGS := $(LDFLAGS) \
	-ltbox_network -ltbox_network_tls \
	-ltbox_log -ltbox_eventx -ltbox_event -ltbox_util -ltbox_base \
	-lssl -lcrypto -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
