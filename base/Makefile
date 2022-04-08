LIB_NAME = base
LIB_NAME_EXT =

LIB_VERSION_X = 1
LIB_VERSION_Y = 0
LIB_VERSION_Z = 0

HEAD_FILES = \
	log.h \
	log_imp.h \
	log_output.h \
	defines.h \
	scope_exit.hpp \
	cabinet.hpp \
	cabinet_token.h \
	memblock.h \
	json_fwd.h \
	json.hpp \

CPP_SRC_FILES = \
	log_imp.cpp \
	log_output.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox_base"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	log_output_test.cpp \
	scope_exit_test.cpp \
	cabinet_test.cpp \
	json_test.cpp \

TEST_LDFLAGS := $(LDFLAGS)

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
