PROJECT = alarm
LIB_NAME = alarm
LIB_VERSION_X = 0
LIB_VERSION_Y = 0
LIB_VERSION_Z = 1

HEAD_FILES = \
	alarm.h \
	weekly_alarm.h \
	oneshot_alarm.h \
	cron_alarm.h \
	workday_alarm.h \
	workday_calendar.h \

CPP_SRC_FILES = \
	alarm.cpp \
	weekly_alarm.cpp \
	oneshot_alarm.cpp \
	cron_alarm.cpp \
	3rd-party/ccronexpr.cpp \
	workday_alarm.cpp \
	workday_calendar.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"tbox.alarm"' $(CXXFLAGS)

TEST_CPP_SRC_FILES = \
	$(CPP_SRC_FILES) \
	workday_calendar_test.cpp \

TEST_LDFLAGS := $(LDFLAGS) -ltbox_event -ltbox_base
ENABLE_SHARED_LIB = no

include ../../tools/lib_tbox_common.mk
