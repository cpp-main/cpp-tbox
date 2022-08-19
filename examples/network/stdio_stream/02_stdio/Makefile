EXE_NAME := example/network/stdio_stream/02_stdio

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_network \
	-ltbox_event \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
