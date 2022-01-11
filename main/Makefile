LIB_NAME = main
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	app.h \
	apps.h \
	context.h \
	main.h \

SRC_FILES = \
	apps.cpp \
	context.cpp \
	main.cpp \
	error_handle.cpp \
	misc.cpp \
	args.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"main"' $(CXXFLAGS)

TEST_SRC_FILES = \
	test_main.cpp \
	args_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_eventx -ltbox_event-ne -ltbox_util -ltbox_base -levent_core -lev

ENABLE_SHARED_LIB = no

include ../tools/lib_common.mk
