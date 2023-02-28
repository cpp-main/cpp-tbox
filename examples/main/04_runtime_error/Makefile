EXE_NAME := example/main/04_runtime_error

CPP_SRC_FILES = \
	main.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_main \
	-ltbox_terminal \
	-ltbox_network \
	-ltbox_eventx \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_log \
	-ltbox_util \
	-ltbox_base \
	-lpthread \
	-ldl \
	-rdynamic

include ${TOP_DIR}/tools/exe_common.mk
