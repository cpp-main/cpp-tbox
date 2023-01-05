if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_terminal.la
else
noinst_LTLIBRARIES = libtbox_terminal.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/terminal

HEAD_FILES = \
			 types.h \
			 connection.h \
			 terminal_interact.h \
			 terminal_nodes.h \
			 session.h \
			 terminal.h \
			 service/telnetd.h \
			 service/tcp_rpc.h

CPP_SRC_FILES = \
				terminal.cpp \
				session.cpp \
				service/telnetd.cpp \
				service/tcp_rpc.cpp \
				impl/terminal.cpp \
				impl/terminal_key_events.cpp \
				impl/terminal_commands.cpp \
				impl/terminal_nodes.cpp \
				impl/key_event_scanner.cpp \
				impl/dir_node.cpp \
				impl/func_node.cpp \
				impl/service/telnetd.cpp \
				impl/service/tcp_rpc.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"terminal"'

libtbox_terminal_la_SOURCES = $(CPP_SRC_FILES)
libtbox_terminal_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_terminal_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
