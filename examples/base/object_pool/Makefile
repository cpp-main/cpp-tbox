PROJECT := example/base/object_pool
EXE_NAME := object_pool

CPP_SRC_FILES := main.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += -ltbox_base

include ${TOP_DIR}/tools/exe_common.mk
