PROJECT = terminal
LIB_NAME = terminal
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	types.h \
	connection.h \
	terminal_interact.h \
	terminal_nodes.h \
	session.h \
	terminal.h \
	service/telnetd.h \
	service/tcp_rpc.h \

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
	impl/service/tcp_rpc.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox.terminal"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	impl/key_event_scanner.cpp \
	impl/key_event_scanner_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_event -ltbox_util -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include ../../tools/lib_tbox_common.mk
