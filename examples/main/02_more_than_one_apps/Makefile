EXE_NAME := example/main/02_more_than_one_apps

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

include app1/app.mk
include app2/app.mk
include app3/app.mk

include ${TOP_DIR}/tools/exe_common.mk
