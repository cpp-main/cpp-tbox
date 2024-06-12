################################################################
# ABOUT:
# This C++ library project Makefile's common part, which can be included
# into project's Makefile, to make Makefile is easy to maintain.
#
# HOW TO USE:
# before include this file. those variables need be specified.
# LIB_NAME, STAGING_DIR, HEAD_FILES
################################################################

.PHONY: all install uninstall

SRC_HEAD_TO_INSTALL_HEAD = $(addprefix $(STAGING_DIR)/include/$(LIB_NAME)/,$(1))

define CREATE_INSTALL_HEAD_TARGET
$(call SRC_HEAD_TO_INSTALL_HEAD,$(1)) : $(1)
	@install -Dm 640 $$^ $$@
endef

$(foreach src,$(HEAD_FILES),$(eval $(call CREATE_INSTALL_HEAD_TARGET,$(src))))

INSTALL_HEADS := $(foreach src,$(HEAD_FILES),$(call SRC_HEAD_TO_INSTALL_HEAD,$(src)))

all : install

install: $(INSTALL_HEADS)

uninstall:
	rm -rf $(STAGING_DIR)/include/$(LIB_NAME)

clean:

