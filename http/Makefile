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

CPP_SRC_FILES = \
	common.cpp \
	request.cpp \
	respond.cpp \
	server.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"http"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	server_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_network -ltbox_log -ltbox_event -ltbox_base

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
