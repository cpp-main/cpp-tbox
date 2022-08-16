EXE_NAME := example/mqtt/pub

CPP_SRC_FILES := pub.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_mqtt \
	-ltbox_event \
	-ltbox_base \
	-lmosquitto \

include ${TOP_DIR}/tools/exe_common.mk
