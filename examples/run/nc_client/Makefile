PROJECT = example/run/nc_client

LIB_NAME = nc_client
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

CPP_SRC_FILES = nc_client.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(LIB_NAME)"' $(CXXFLAGS)
LDFLAGS += -ltbox_network

ENABLE_STATIC_LIB = no
ENABLE_SHARED_LIB = yes

include ${TOP_DIR}/tools/lib_common.mk
