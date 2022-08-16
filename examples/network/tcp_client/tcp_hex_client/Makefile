EXE_NAME := example/network/tcp_client/tcp_hex_client

CPP_SRC_FILES := tcp_hex_client.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_network \
	-ltbox_event \
	-ltbox_util \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
