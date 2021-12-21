include build_env.mk

.PHONY: all clean distclean

CCFLAGS := -Wall -Werror

ifeq ($(RELEASE), 1)
CCFLAGS += -O2
else
CCFLAGS += -fsanitize=address -fno-omit-frame-pointer -DDEBUG=1 -O0 -ggdb
LDFLAGS += -fsanitize=address -static-libasan
endif

CXXFLAGS := $(CCFLAGS) $(CXXFLAGS)
CFLAGS := $(CCFLAGS) $(CFLAGS)
APPS_DIR := $(PWD)

export CC CXX CFLAGS CXXFLAGS LDFLAGS APPS_DIR

app_y += base
app_y += event
app_y += eventx
app_y += network
app_y += coroutine
app_y += mqtt
app_y += main

all test: $(STAGING_DIR)
	@for i in $(app_y); do \
		[ ! -d $$i ] || $(MAKE) -C $$i $@ || exit $$? ; \
	done

$(STAGING_DIR):
	if [ ! -d $(STAGING_DIR) ]; then \
		mkdir -p $(STAGING_DIR)/include; \
		mkdir -p $(STAGING_DIR)/lib; \
	fi

clean:
	@for i in $(app_y); do \
		[ ! -d $$i ] || $(MAKE) -C $$i $@ || exit $$? ; \
	done

distclean:
	@for i in $(app_y); do \
		[ ! -d $$i ] || $(MAKE) -C $$i $@ || exit $$? ; \
	done
	-rm -rf $(STAGING_DIR)
