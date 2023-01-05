if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_network.la
else
noinst_LTLIBRARIES = libtbox_network.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/network

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
			 net_if.h

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
				net_if.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"network"'

libtbox_network_la_SOURCES = $(CPP_SRC_FILES)
libtbox_network_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_network_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
