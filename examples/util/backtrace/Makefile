EXE_NAME := example/util/backstrace

CPP_SRC_FILES := sample.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_util \
	-ltbox_base \
	-ldl \

include ${TOP_DIR}/tools/exe_common.mk
