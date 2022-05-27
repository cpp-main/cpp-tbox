LIB_NAME = network
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	buffer.h \
	fd.h \
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
	ssl.h \
	ssl_ctx.h \
	tcp_ssl_connection.h \
	tcp_ssl_acceptor.h \
	tcp_ssl_connector.h \
	tcp_ssl_client.h \
	tcp_ssl_server.h \

CPP_SRC_FILES = \
	buffer.cpp \
	fd.cpp \
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
	ssl.cpp \
	ssl_ctx.cpp \
	tcp_ssl_immature_connection.cpp \
	tcp_ssl_connection.cpp \
	tcp_ssl_acceptor.cpp \
	tcp_ssl_connector.cpp \
	tcp_ssl_client.cpp \
	tcp_ssl_server.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"network"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	fd_test.cpp \
	buffer_test.cpp \
	buffered_fd_test.cpp \
	uart_test.cpp \
	ip_address_test.cpp \
	sockaddr_test.cpp \
	udp_socket_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
