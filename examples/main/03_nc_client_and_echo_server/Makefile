PROJECT := example/main/03_nc_client_and_echo_server
EXE_NAME := 03_nc_client_and_echo_server

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

include echo_server/app.mk
include nc_client/app.mk

include ${TOP_DIR}/tools/exe_common.mk
