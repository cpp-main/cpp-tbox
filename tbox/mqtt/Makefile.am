if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_mqtt.la
else
noinst_LTLIBRARIES = libtbox_mqtt.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/mqtt

HEAD_FILES = client.h
CPP_SRC_FILES = client.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"mqtt"'

libtbox_mqtt_la_SOURCES = $(CPP_SRC_FILES)
libtbox_mqtt_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_mqtt_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
