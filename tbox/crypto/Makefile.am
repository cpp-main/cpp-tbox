if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_crypto.la
else
noinst_LTLIBRARIES = libtbox_crypto.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/crypto

HEAD_FILES = \
			 md5.h \
			 aes.h

CPP_SRC_FILES = \
				md5.cpp \
				aes.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"tbox_crypto"'

libtbox_crypto_la_SOURCES = $(CPP_SRC_FILES)
libtbox_crypto_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_crypto_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
