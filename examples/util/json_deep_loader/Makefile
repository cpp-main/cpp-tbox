PROJECT := example/util/json_deep_loader
EXE_NAME := json_deep_loader

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_util \
	-ltbox_base \
	-ldl \

include ${TOP_DIR}/tools/exe_common.mk
