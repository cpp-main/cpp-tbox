PROJECT := example/event/04_run_in_loop
EXE_NAME := 04_run_in_loop

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_event \
	-ltbox_base \
	-lpthread \

include ${TOP_DIR}/tools/exe_common.mk
