PROJECT := example/network/udp_socket/recv_only
EXE_NAME := recv_only

CPP_SRC_FILES := recv_only.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_network \
	-ltbox_event \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
