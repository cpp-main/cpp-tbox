all test clean distclean:
	@for i in $(shell ls); do \
		[ -d $$i ] && ( $(MAKE) -C $$i $@ || exit $$? ) || exit 0; \
	done
