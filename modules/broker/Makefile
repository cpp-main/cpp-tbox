LIB_NAME = broker
LIB_VERSION_X = 0 
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	broker.h \

CPP_SRC_FILES = \
	broker.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"broker"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_common.mk
