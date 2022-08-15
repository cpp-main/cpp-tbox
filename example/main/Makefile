subdirs := $(shell find -maxdepth 1 -type d | grep -v '.')

all test clean distclean:
	@for i in $(subdirs); do \
		$(MAKE) -C $$i $@ || exit $$? ; \
	done
