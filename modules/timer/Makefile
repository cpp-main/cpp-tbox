LIB_NAME = timer
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	timer.h \
	weekly_timer.h \
	oneshot_timer.h \
	cron_timer.h \

CPP_SRC_FILES = \
	timer.cpp \
	weekly_timer.cpp \
	oneshot_timer.cpp \
	cron_timer.cpp \
	ccronexpr.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"timer"' $(CXXFLAGS)

TEST_CPP_SRC_FILES =

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_common.mk
