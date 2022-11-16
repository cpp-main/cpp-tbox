EXE_NAME := example/main/05_loop_block

CPP_SRC_FILES = \
	app.cpp \
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
