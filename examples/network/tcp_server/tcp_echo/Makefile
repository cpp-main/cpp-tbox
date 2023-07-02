PROJECT := example/network/tcp_server/tcp_echo
EXE_NAME := tcp_echo

CPP_SRC_FILES := tcp_echo.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_network \
	-ltbox_event \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
