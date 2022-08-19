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
	server/router.h \
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
	server/router.cpp \
	client/client.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"http"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	common_test.cpp \
	respond_test.cpp \
	request_test.cpp \
	url_test.cpp \
	server/request_parser_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_log -ltbox_event -ltbox_util -ltbox_base

ENABLE_SHARED_LIB = no

include ../../tools/lib_common.mk
