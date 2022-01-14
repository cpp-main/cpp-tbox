EXE_NAME = nc_server_client

CPP_SRC_FILES = \

TEST_CPP_SRC_FILES = \

CXXFLAGS := -DLOG_MODULE_ID='"sample"' $(CXXFLAGS)
LDFLAGS += -L.. \
	-ltbox_main \
	-ltbox_eventx \
	-ltbox_event-ne \
	-ltbox_util \
	-ltbox_base \
	-levent_core \
	-lev -lpthread

TEST_LDFLAGS := $(LDFLAGS)

include nc_client/app.mk
include echo_server/app.mk

include ../tools/exe_common.mk
