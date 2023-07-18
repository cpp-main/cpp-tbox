PROJECT := example/http/server/router
EXE_NAME := router

CPP_SRC_FILES := router.cpp

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_http \
	-ltbox_network \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_log \
	-ltbox_util \
	-ltbox_base \
	-lpthread

include ${TOP_DIR}/tools/exe_common.mk
