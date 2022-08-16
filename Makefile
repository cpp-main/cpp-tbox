include build_env.mk

.PHONY: all clean distclean

CCFLAGS := -Wall

ifeq ($(RELEASE), 1)
CCFLAGS += -O2 -Os
else
CCFLAGS += -DDEBUG=1 -O0 -ggdb
endif

ifeq ($(ENABLE_ASAN), 1)
CCFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan
endif

ifeq ($(ENABLE_GPROF), 1)
CCFLAGS += -pg
LDFLAGS += -pg
endif

CXXFLAGS := $(CCFLAGS) $(CXXFLAGS)
CFLAGS := $(CCFLAGS) $(CFLAGS)
APPS_DIR := $(PWD)

export CC CXX CFLAGS CXXFLAGS LDFLAGS APPS_DIR

include config.mk

all test:
	@for i in $(app_y); do \
		[ ! -d $$i ] || $(MAKE) -C $$i $@ || exit $$? ; \
	done

clean:
	-rm -rf $(OUTPUT_DIR)

distclean: clean
	-rm -rf $(STAGING_DIR) $(INSTALL_DIR)
