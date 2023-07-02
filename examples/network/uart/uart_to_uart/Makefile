PROJECT := example/network/uart/uart_to_uart
EXE_NAME := uart_to_uart

CPP_SRC_FILES := uart_to_uart.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_network \
	-ltbox_event \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
