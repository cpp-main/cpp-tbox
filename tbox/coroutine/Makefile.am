if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_coroutine.la
else
noinst_LTLIBRARIES = libtbox_coroutine.la
endif

MAINTAINERCLEANFILES = Makefile.in

EVENT_CXXFLAGS =
VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/coroutine

HEAD_FILES = \
	scheduler.h \
	channel.hpp \
	semaphore.hpp \
	mutex.hpp \
	broadcast.hpp \
	condition.hpp

CPP_SRC_FILES = \
	scheduler.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"coroutine"'

libtbox_coroutine_la_SOURCES = $(CPP_SRC_FILES)
libtbox_coroutine_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_coroutine_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)

if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
