LIB_NAME = main
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 2

HEAD_FILES = \
	context.h \
	main.h \
	module.h \

CPP_SRC_FILES = \
	context_imp.cpp \
	run_in_frontend.cpp \
	run_in_backend.cpp \
	signal.cpp \
	misc.cpp \
	args.cpp \
	module.cpp \
	log.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox.main"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS) \
	-ltbox_terminal \
	-ltbox_network \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_log \
	-ltbox_util \
	-ltbox_base \
	-ldl \

ENABLE_SHARED_LIB = no

include ../../tools/lib_tbox_common.mk
