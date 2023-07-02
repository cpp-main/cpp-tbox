PROJECT := example/mqtt/pub
EXE_NAME := pub

CPP_SRC_FILES := pub.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_mqtt \
	-ltbox_event \
	-ltbox_base \
	-lmosquitto \
	-lpthread \

include ${TOP_DIR}/tools/exe_common.mk
