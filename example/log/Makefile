subdirs := $(shell ls | grep -v Makefile)

all test clean distclean:
	@for i in $(subdirs); do \
		$(MAKE) -C $$i $@ || exit $$? ; \
	done
