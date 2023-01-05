if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_eventx.la
else
noinst_LTLIBRARIES = libtbox_eventx.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/eventx

HEAD_FILES =  \
			  thread_pool.h \
			  timer_pool.h \
			  timeout_monitor.h \
			  request_pool.hpp \
			  loop_wdog.h

CPP_SRC_FILES = \
				thread_pool.cpp \
				timer_pool.cpp \
				timeout_monitor.cpp \
				loop_wdog.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"eventx"'

libtbox_eventx_la_SOURCES = $(CPP_SRC_FILES)
libtbox_eventx_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_eventx_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
