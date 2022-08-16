EXE_NAME := example/eventx/thread-pool

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_base \
	-lpthread \

include ${TOP_DIR}/tools/exe_common.mk
