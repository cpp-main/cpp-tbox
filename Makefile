include build_env.mk

.PHONY: all clean distclean

CCFLAGS := -Wall -Werror
CXXFLAGS := $(CCFLAGS) $(CXXFLAGS)
CFLAGS := $(CCFLAGS) $(CFLAGS)
APPS_DIR := $(PWD)

export CC CXX CFLAGS CXXFLAGS LDFLAGS APPS_DIR

app_y += base

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
