PROJECT := example/network/tcp_acceptor/tcp_nc_server
EXE_NAME := tcp_nc_server

CPP_SRC_FILES := tcp_nc_server.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_network \
	-ltbox_event \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
