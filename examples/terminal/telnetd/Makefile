PROJECT := example/terminal/telnetd
EXE_NAME := telnetd

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_terminal \
	-ltbox_network \
	-ltbox_event \
	-ltbox_util \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
