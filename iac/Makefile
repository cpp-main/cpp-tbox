LIB_NAME = iac
LIB_VERSION_X = 0 
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	broker.h \
	service.h \

CPP_SRC_FILES = \
	broker.cpp \
	service.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"iac"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event-ne -ltbox_base -levent_core -lev
ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
