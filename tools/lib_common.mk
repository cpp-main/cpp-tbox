################################################################
# ABOUT:
# This C++ library project Makefile's common part, which can be included
# into project's Makefile, to make Makefile is easy to maintain.
#
# HOW TO USE:
# before include this file. those variables need be specified.
# LIB_VERSION_X, LIB_VERSION_Y, LIB_VERSION_Z, C_SRC_FILES, CC_SRC_FILES, CPP_SRC_FILES, CXXFLAGS, TEST_LDFLAGS
#
# TARGETS:
# all, libxxx.a, libxxx.so.x.x.x, test, clean, distclean, install, uninstall
################################################################

.PHONY: all print_vars print_static_vars print_shared_vars print_test_vars clean distclean install uninstall pre_build post_build

LIB_BASENAME = libtbox_$(LIB_NAME)$(LIB_NAME_EXT)
LIB_OUTPUT_DIR = $(OUTPUT_DIR)/$(LIB_NAME)

STATIC_LIB := $(LIB_BASENAME).a
SHARED_LIB := $(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y).$(LIB_VERSION_Z)

ENABLE_STATIC_LIB ?= yes
ENABLE_SHARED_LIB ?= yes

TARGETS := pre_build print_vars

ifeq ($(ENABLE_STATIC_LIB),yes)
	TARGETS += $(LIB_OUTPUT_DIR)/$(STATIC_LIB)
endif

ifeq ($(ENABLE_SHARED_LIB),yes)
	TARGETS += $(LIB_OUTPUT_DIR)/$(SHARED_LIB)
endif

TARGETS += post_build

all: $(TARGETS) install

print_vars:
	@echo CXX=$(CXX)

################################################################
# static library
################################################################

CPP_SOURCE_TO_OBJECT = $(LIB_OUTPUT_DIR)/$(subst .cpp,.o,$(1))
CC_SOURCE_TO_OBJECT = $(LIB_OUTPUT_DIR)/$(subst .cc,.o,$(1))
C_SOURCE_TO_OBJECT = $(LIB_OUTPUT_DIR)/$(subst .c,.o,$(1))

OBJECTS := $(foreach src,$(CPP_SRC_FILES),$(call CPP_SOURCE_TO_OBJECT,$(src)))
OBJECTS += $(foreach src,$(CC_SRC_FILES),$(call CC_SOURCE_TO_OBJECT,$(src)))
OBJECTS += $(foreach src,$(C_SRC_FILES),$(call C_SOURCE_TO_OBJECT,$(src)))

define CREATE_CPP_OBJECT
$(call CPP_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@mkdir -p $$(dir $$@)
	@$(CXX) $(CXXFLAGS) -o $$@ -c $$^
endef

define CREATE_CC_OBJECT
$(call CC_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@mkdir -p $$(dir $$@)
	@$(CXX) $(CXXFLAGS) -o $$@ -c $$^
endef

define CREATE_C_OBJECT
$(call C_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $^\033[0m"
	@$(CC) $(CFLAGS) -o $@ -c $^
endef

$(foreach src,$(CPP_SRC_FILES),$(eval $(call CREATE_CPP_OBJECT,$(src))))
$(foreach src,$(CC_SRC_FILES),$(eval $(call CREATE_CC_OBJECT,$(src))))
$(foreach src,$(C_SRC_FILES),$(eval $(call CREATE_C_OBJECT,$(src))))

print_static_vars :
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo LDFLAGS=$(LDFLAGS)
	@echo OBJECTS=$(OBJECTS)

$(LIB_OUTPUT_DIR)/$(STATIC_LIB) : print_static_vars $(OBJECTS)
	@echo "\033[35mBUILD $(STATIC_LIB) \033[0m"
	@mkdir -p $(dir $@)
	@$(AR) rc $@ $(OBJECTS)

################################################################
# shared library
################################################################

CPP_SOURCE_TO_SHARED_OBJECT = $(LIB_OUTPUT_DIR)/$(subst .cpp,.oS,$(1))
CC_SOURCE_TO_SHARED_OBJECT = $(LIB_OUTPUT_DIR)/$(subst .cc,.oS,$(1))
C_SOURCE_TO_SHARED_OBJECT = $(LIB_OUTPUT_DIR)/$(subst .c,.oS,$(1))

SHARED_OBJECTS := $(foreach src,$(CPP_SRC_FILES),$(call CPP_SOURCE_TO_SHARED_OBJECT,$(src)))
SHARED_OBJECTS += $(foreach src,$(CC_SRC_FILES),$(call CC_SOURCE_TO_SHARED_OBJECT,$(src)))
SHARED_OBJECTS += $(foreach src,$(C_SRC_FILES),$(call C_SOURCE_TO_SHARED_OBJECT,$(src)))

SHARED_CXXFLAGS := $(CXXFLAGS) -fPIC
SHARED_CFLAGS := $(CFLAGS) -fPIC

define CREATE_CPP_SHARED_OBJECT
$(call CPP_SOURCE_TO_SHARED_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@mkdir -p $$(dir $$@)
	@$(CXX) $(SHARED_CXXFLAGS) -o $$@ -c $$^
endef

define CREATE_CC_SHARED_OBJECT
$(call CC_SOURCE_TO_SHARED_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@mkdir -p $$(dir $$@)
	@$(CXX) $(SHARED_CXXFLAGS) -o $$@ -c $$^
endef

define CREATE_C_SHARED_OBJECT
$(call C_SOURCE_TO_SHARED_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $^\033[0m"
	@$(CC) $(SHARED_CFLAGS) -o $@ -c $^
endef

$(foreach src,$(CPP_SRC_FILES),$(eval $(call CREATE_CPP_SHARED_OBJECT,$(src))))
$(foreach src,$(CC_SRC_FILES),$(eval $(call CREATE_CC_SHARED_OBJECT,$(src))))
$(foreach src,$(C_SRC_FILES),$(eval $(call CREATE_C_SHARED_OBJECT,$(src))))

print_shared_vars :
	@echo SHARED_LIB=$(SHARED_LIB)
	@echo SHARED_CXXFLAGS=$(SHARED_CXXFLAGS)
	@echo SHARED_OBJECTS=$(SHARED_OBJECTS)

$(LIB_OUTPUT_DIR)/$(SHARED_LIB) : print_shared_vars $(SHARED_OBJECTS)
	-rm -f $(LIB_OUTPUT_DIR)/$(LIB_BASENAME).so*
	@echo "\033[35mBUILD $(SHARED_LIB)\033[0m"
	@$(CXX) -shared $(SHARED_OBJECTS) -Wl,-soname,$(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y) -o $@
	cd $(LIB_OUTPUT_DIR); \
	ln -s $(SHARED_LIB) $(LIB_BASENAME).so; \
	ln -s $(SHARED_LIB) $(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y)

################################################################
# test
################################################################

CPP_SOURCE_TO_TEST_OBJECT = $(LIB_OUTPUT_DIR)/$(subst .cpp,.oT,$(1))
C_SOURCE_TO_TEST_OBJECT = $(LIB_OUTPUT_DIR)/$(subst .c,.oT,$(1))

TEST_OBJECTS := $(foreach src,$(TEST_CPP_SRC_FILES),$(call CPP_SOURCE_TO_TEST_OBJECT,$(src)))
TEST_CXXFLAGS := $(CXXFLAGS)

define CREATE_CPP_TEST_OBJECT
$(call CPP_SOURCE_TO_TEST_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@mkdir -p $$(dir $$@)
	@$(CXX) $(TEST_CXXFLAGS) -o $$@ -c $$^
endef

$(foreach src,$(TEST_CPP_SRC_FILES),$(eval $(call CREATE_CPP_TEST_OBJECT,$(src))))

test : $(LIB_OUTPUT_DIR)/test

print_test_vars :
	@echo TEST_OBJECTS=$(TEST_OBJECTS)
	@echo TEST_CXXFLAGS=$(TEST_CXXFLAGS)
	@echo TEST_LDFLAGS=$(TEST_LDFLAGS)

$(LIB_OUTPUT_DIR)/test: print_test_vars $(TEST_OBJECTS) $(OBJECTS)
	@echo "\033[35mBUILD test\033[0m"
	@$(CXX) -o $@ $(TEST_OBJECTS) $(OBJECTS) $(TEST_LDFLAGS) -lgmock_main -lgmock -lgtest -lpthread

# head files
HEAD_TO_DEST_FILE = $(addprefix $(STAGING_DIR)/include/tbox/$(LIB_NAME)/,$(1))

define CREATE_HEAD_FILE_TARGET
$(call HEAD_TO_DEST_FILE,$(1)) : $(1)
	@mkdir -p $$(dir $$@)
	@echo "\033[35mCOPY $(1)\033[0m"
	@cp $$^ $$@
endef

$(foreach src,$(HEAD_FILES),$(eval $(call CREATE_HEAD_FILE_TARGET,$(src))))

HEAD_FILE_TARGETS := $(foreach src,$(HEAD_FILES),$(call HEAD_TO_DEST_FILE,$(src)))

################################################################
# clean and distclean
################################################################
clean:
	-rm -f $(OBJECTS) $(SHARED_OBJECTS) $(TEST_OBJECTS)

################################################################
# install and uninstall
################################################################

install: $(HEAD_FILE_TARGETS)
ifeq ($(ENABLE_STATIC_LIB),yes)
	cp $(LIB_OUTPUT_DIR)/$(STATIC_LIB) $(STAGING_DIR)/lib/
endif
ifeq ($(ENABLE_SHARED_LIB),yes)
	cp -d $(LIB_OUTPUT_DIR)/$(LIB_BASENAME).so* $(STAGING_DIR)/lib/
endif

uninstall:
	rm -rf $(STAGING_DIR)/include/$(LIB_NAME)
	rm -f $(STAGING_DIR)/lib/$(LIB_BASENAME).*
