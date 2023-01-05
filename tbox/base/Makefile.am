if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_base.la
else
noinst_LTLIBRARIES = libtbox_base.la
endif

MAINTAINERCLEANFILES = Makefile.in

includedir = @includedir@/tbox/base

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

HEAD_FILES = log.h \
			 log_imp.h \
			 log_output.h \
			 defines.h \
			 scope_exit.hpp \
			 cabinet.hpp \
			 cabinet_token.h \
			 memblock.h \
			 json_fwd.h \
			 json.hpp \
			 assert.h \
			 lifetime_tag.hpp

CPP_SRC_FILES = log_imp.cpp \
				log_output.cpp

libtbox_base_la_SOURCES = $(CPP_SRC_FILES)
libtbox_base_la_CXXFLAGS = -DLOG_MODULE_ID='"tbox_base"'
libtbox_base_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)

if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
