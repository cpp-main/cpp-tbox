LIB_NAME = http
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	common.h \
	middleware.h \
	request.h \
	respond.h \
	server.h \
	context.h \

CPP_SRC_FILES = \
	common.cpp \
	request.cpp \
	request_parser.cpp \
	respond.cpp \
	server.cpp \
	server_imp.cpp \
	context.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"http"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	server_test.cpp \
	common_test.cpp \
	respond_test.cpp \
	request_test.cpp \
	request_parser_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_log -ltbox_event -ltbox_base

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
