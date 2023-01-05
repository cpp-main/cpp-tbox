if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_util.la
else
noinst_LTLIBRARIES = libtbox_util.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/util

HEAD_FILES = \
			 pid_file.h \
			 string_util.h \
			 fs.h \
			 argument_parser.h \
			 split_cmdline.h \
			 serializer.h \
			 time_counter.h \
			 async_pipe.h \
			 backtrace.h \
			 timestamp.h \
			 json.h \
			 base64.h \
			 checksum.h \
			 crc.h

CPP_SRC_FILES = \
				pid_file.cpp \
				string_util.cpp \
				fs.cpp \
				argument_parser.cpp \
				split_cmdline.cpp \
				serializer.cpp \
				time_counter.cpp \
				async_pipe.cpp \
				backtrace.cpp \
				timestamp.cpp \
				json.cpp \
				base64.cpp \
				checksum.cpp \
				crc.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"util"'

libtbox_util_la_SOURCES = $(CPP_SRC_FILES)
libtbox_util_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_util_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
