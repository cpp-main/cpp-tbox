EXE_NAME := sample

CPP_SRC_FILES := app_main.cpp build_time.cpp
TEST_CPP_SRC_FILES :=
TEST_LDFLAGS :=

CONF_FILES := default.conf

include app1/app.mk
include app2/app.mk

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

TEST_LDFLAGS += $(LDFLAGS)

pre_build :
	touch build_time.cpp

include ../tools/exe_common.mk
