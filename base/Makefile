LIB_NAME = base
LIB_NAME_EXT =

LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 0

HEAD_FILES = \
	log.h \
	log_output.h \
	defines.h \

SRC_FILES = \
	log_output.cpp \

CCXXFLAGS += -DLOG_MODULE_ID='"tbox_base"' $(CXXFLAGS)

TEST_SRC_FILES = \
	test.cpp \

TEST_LDFLAGS +=

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
