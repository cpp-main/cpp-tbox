PROJECT := example/network/buffered_fd/01-io-echo
EXE_NAME := 01-io-echo

CPP_SRC_FILES := io_echo.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_network \
	-ltbox_event \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
