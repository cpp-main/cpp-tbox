if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_log.la
else
noinst_LTLIBRARIES = libtbox_log.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/log

HEAD_FILES = \
			 channel.h \
			 async_channel.h \
			 stdout_channel.h \
			 syslog_channel.h \
			 file_async_channel.h

CPP_SRC_FILES = \
				channel.cpp \
				async_channel.cpp \
				stdout_channel.cpp \
				syslog_channel.cpp \
				file_async_channel.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"log"'

libtbox_log_la_SOURCES = $(CPP_SRC_FILES)
libtbox_log_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_log_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
