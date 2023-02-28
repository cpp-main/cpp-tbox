LIB_NAME = nlohmann

HEAD_FILES = \
	json.hpp \
	json_fwd.hpp \

##
SRC_HEAD_TO_INSTALL_HEAD = $(addprefix $(STAGING_DIR)/include/$(LIB_NAME)/,$(1))

define CREATE_INSTALL_HEAD_TARGET
$(call SRC_HEAD_TO_INSTALL_HEAD,$(1)) : $(1)
	@install -Dm 640 $$^ $$@
endef

$(foreach src,$(HEAD_FILES),$(eval $(call CREATE_INSTALL_HEAD_TARGET,$(src))))

INSTALL_HEADS := $(foreach src,$(HEAD_FILES),$(call SRC_HEAD_TO_INSTALL_HEAD,$(src)))

all : $(INSTALL_HEADS)
