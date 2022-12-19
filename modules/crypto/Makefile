LIB_NAME = crypto
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	md5.h \
	aes.h \

CPP_SRC_FILES = \
	md5.cpp \
	aes.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox_crypto"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	md5_test.cpp \
	aes_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_util -ltbox_base

ENABLE_SHARED_LIB = no

include ../../tools/lib_common.mk
