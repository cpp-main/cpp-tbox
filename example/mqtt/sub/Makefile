EXE_NAME := example/mqtt/sub

CPP_SRC_FILES := sub.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_mqtt \
	-ltbox_event \
	-ltbox_base \
	-lmosquitto \
	-lpthread \

include ${TOP_DIR}/tools/exe_common.mk
