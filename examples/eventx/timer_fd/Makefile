PROJECT := example/eventx/timer_fd
EXE_NAME := timer_fd

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_base \
	-ldl

include ${TOP_DIR}/tools/exe_common.mk
