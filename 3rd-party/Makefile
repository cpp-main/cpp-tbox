all test:
	@for i in $(THIRDPARTY); do \
		[ ! -d $$i ] || $(MAKE) -C $$i $@ || exit $$? ; \
	done
