PROJECT = example/run/echo_server

LIB_NAME = echo_server
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

CPP_SRC_FILES = echo_server.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(LIB_NAME)"' $(CXXFLAGS)

ENABLE_STATIC_LIB = no
ENABLE_SHARED_LIB = yes

include ${TOP_DIR}/tools/lib_common.mk
