if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_alarm.la
else
noinst_LTLIBRARIES = libtbox_alarm.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/alarm

HEAD_FILES = alarm.h \
			 weekly_alarm.h \
			 oneshot_alarm.h \
			 cron_alarm.h \
			 workday_alarm.h \
			 workday_calendar.h

CPP_SRC_FILES = alarm.cpp \
				weekly_alarm.cpp \
				oneshot_alarm.cpp \
				cron_alarm.cpp \
				3rd-party/ccronexpr.cpp \
				workday_alarm.cpp \
				workday_calendar.cpp

libtbox_alarm_la_SOURCES = $(CPP_SRC_FILES)
libtbox_alarm_la_CXXFLAGS = -DLOG_MODULE_ID='"alarm"'
libtbox_alarm_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
