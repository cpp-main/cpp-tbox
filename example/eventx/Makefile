all test clean distclean:
	@for i in $(shell ls) ; do \
		if [ -d $$i ]; then  \
			$(MAKE) -C $$i $@ || exit $$? ; \
		fi \
	done
