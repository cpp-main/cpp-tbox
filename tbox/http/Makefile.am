if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_http.la
else
noinst_LTLIBRARIES = libtbox_http.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:0
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/http

HEAD_FILES = \
			 common.h \
			 request.h \
			 respond.h \
			 url.h \
			 server/types.h \
			 server/server.h \
			 server/context.h \
			 server/middleware.h \
			 server/router.h \
			 client/client.h

CPP_SRC_FILES = \
				common.cpp \
				request.cpp \
				respond.cpp \
				url.cpp \
				server/request_parser.cpp \
				server/server.cpp \
				server/server_imp.cpp \
				server/context.cpp \
				server/router.cpp \
				client/client.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"http"'

libtbox_http_la_SOURCES = $(CPP_SRC_FILES)
libtbox_http_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_http_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
