if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_main.la
else
noinst_LTLIBRARIES = libtbox_main.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/main

HEAD_FILES = \
			 context.h \
			 main.h \
			 module.h

CPP_SRC_FILES = \
				context_imp.cpp \
				run_in_frontend.cpp \
				run_in_backend.cpp \
				signal.cpp \
				misc.cpp \
				args.cpp \
				module.cpp \
				log.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"main"'

libtbox_main_la_SOURCES = $(CPP_SRC_FILES)
libtbox_main_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_main_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
