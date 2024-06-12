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

PROJECT = network
LIB_NAME = network
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	buffered_fd.h \
	byte_stream.h \
	stdio_stream.h \
	uart.h \
	socket_fd.h \
	ip_address.h \
	sockaddr.h \
	udp_socket.h \
	tcp_connection.h \
	tcp_acceptor.h \
	tcp_connector.h \
	tcp_client.h \
	tcp_server.h \
	net_if.h \
	domain_name.h \
	dns_request.h \

CPP_SRC_FILES = \
	buffered_fd.cpp \
	stdio_stream.cpp \
	uart.cpp \
	socket_fd.cpp \
	ip_address.cpp \
	sockaddr.cpp \
	udp_socket.cpp \
	tcp_connection.cpp \
	tcp_acceptor.cpp \
	tcp_connector.cpp \
	tcp_client.cpp \
	tcp_server.cpp \
	net_if.cpp \
	dns_request.cpp \

CXXFLAGS := -DMODULE_ID='"tbox.network"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	buffered_fd_test.cpp \
	uart_test.cpp \
	ip_address_test.cpp \
	sockaddr_test.cpp \
	udp_socket_test.cpp \
	net_if_test.cpp \
	dns_request_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_eventx -ltbox_event -ltbox_util -ltbox_base -ldl

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/mk/lib_tbox_common.mk
