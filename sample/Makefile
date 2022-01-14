EXE_NAME := sample

CPP_SRC_FILES := main.cpp build_time.cpp
TEST_CPP_SRC_FILES :=
TEST_LDFLAGS :=

CONF_FILES := default.conf

include app1/app.mk
include app2/app.mk

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_main \
	-ltbox_eventx \
	-ltbox_event-ne \
	-ltbox_util \
	-ltbox_base \
	-levent_core \
	-lev -lpthread

TEST_LDFLAGS += $(LDFLAGS)

pre_build :
	touch build_time.cpp

include ../tools/exe_common.mk
