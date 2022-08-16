LIB_NAME = main
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	context.h \
	main.h \
	module.h \

CPP_SRC_FILES = \
	context_imp.cpp \
	main.cpp \
	signal.cpp \
	misc.cpp \
	args.cpp \
	module.cpp \
	log.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"main"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \

TEST_LDFLAGS := $(LDFLAGS) \
	-ltbox_terminal \
	-ltbox_network \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_log \
	-ltbox_util \
	-ltbox_base \
	-levent_core -lev

ENABLE_SHARED_LIB = no

include $(TOP_DIR)/tools/lib_common.mk
