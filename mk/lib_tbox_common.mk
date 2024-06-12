################################################################
# ABOUT:
# This C++ library project Makefile's common part, which can be included
# into project's Makefile, to make Makefile is easy to maintain.
#
# HOW TO USE:
# before include this file. those variables need be specified.
# PROJECT, LIB_NAME, LIB_NAME_EXT, LIB_VERSION_X, LIB_VERSION_Y, LIB_VERSION_Z,
# C_SRC_FILES, CC_SRC_FILES, CPP_SRC_FILES, CXXFLAGS, LDFLAGS, TEST_LDFLAGS
# BUILD_DIR, STAGING_DIR, INSTALL_DIR
#
# TARGETS:
# all, libxxx.a, libxxx.so.x.x.x, test, clean, distclean, install, uninstall
################################################################

.PHONY: all print_vars print_static_vars print_shared_vars print_test_vars clean distclean install uninstall pre_build post_build

LIB_BASENAME = libtbox_$(LIB_NAME)$(LIB_NAME_EXT)
LIB_BUILD_DIR = $(BUILD_DIR)/$(PROJECT)

STATIC_LIB := $(LIB_BASENAME).a
SHARED_LIB := $(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y).$(LIB_VERSION_Z)

ENABLE_STATIC_LIB ?= yes
ENABLE_SHARED_LIB ?= yes

TARGETS := pre_build #print_vars

ifeq ($(ENABLE_STATIC_LIB),yes)
	TARGETS += build_static_lib
endif

ifeq ($(ENABLE_SHARED_LIB),yes)
	TARGETS += build_shared_lib
endif

TARGETS += post_build

all: $(TARGETS) install

print_vars:
	@echo CXX=$(CXX)
	@echo C_SRC_FILES=$(C_SRC_FILES)
	@echo CC_SRC_FILES=$(CC_SRC_FILES)
	@echo CPP_SRC_FILES=$(CPP_SRC_FILES)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo STAGING_DIR=$(STAGING_DIR)
	@echo INSTALL_DIR=$(INSTALL_DIR)

################################################################
# static library
################################################################

CPP_SOURCE_TO_OBJECT = $(LIB_BUILD_DIR)/$(subst .cpp,.o,$(1))
CC_SOURCE_TO_OBJECT = $(LIB_BUILD_DIR)/$(subst .cc,.o,$(1))
C_SOURCE_TO_OBJECT = $(LIB_BUILD_DIR)/$(subst .c,.o,$(1))

OBJECTS := $(foreach src,$(CPP_SRC_FILES),$(call CPP_SOURCE_TO_OBJECT,$(src)))
OBJECTS += $(foreach src,$(CC_SRC_FILES),$(call CC_SOURCE_TO_OBJECT,$(src)))
OBJECTS += $(foreach src,$(C_SRC_FILES),$(call C_SOURCE_TO_OBJECT,$(src)))

define CREATE_CPP_OBJECT
$(call CPP_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@install -d $$(dir $$@)
	@$(CXX) $(CXXFLAGS) -fPIC -o $$@ -c $$^
endef

define CREATE_CC_OBJECT
$(call CC_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@install -d $$(dir $$@)
	@$(CXX) $(CXXFLAGS) -fPIC -o $$@ -c $$^
endef

define CREATE_C_OBJECT
$(call C_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCC $$^\033[0m"
	@install -d $$(dir $$@)
	@$(CC) $(CFLAGS) -fPIC -o $$@ -c $$^
endef

$(foreach src,$(CPP_SRC_FILES),$(eval $(call CREATE_CPP_OBJECT,$(src))))
$(foreach src,$(CC_SRC_FILES),$(eval $(call CREATE_CC_OBJECT,$(src))))
$(foreach src,$(C_SRC_FILES),$(eval $(call CREATE_C_OBJECT,$(src))))

print_static_vars :
	@echo CFLAGS=$(CFLAGS)
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo OBJECTS=$(OBJECTS)

$(LIB_BUILD_DIR)/$(STATIC_LIB) : $(OBJECTS)
	@echo "\033[35mBUILD $(STATIC_LIB) \033[0m"
	@install -d $(dir $@)
	@$(AR) rc $@ $(OBJECTS)

$(LIB_BUILD_DIR)/$(SHARED_LIB) : $(OBJECTS)
	@echo "\033[35mBUILD $(SHARED_LIB)\033[0m"
	@install -d $(dir $@)
	@$(CXX) -shared $(OBJECTS) -Wl,-soname,$(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y) -o $@ $(LDFLAGS)

build_static_lib : $(LIB_BUILD_DIR)/$(STATIC_LIB)

build_shared_lib : $(LIB_BUILD_DIR)/$(SHARED_LIB)

################################################################
# test
################################################################

CPP_SOURCE_TO_TEST_OBJECT = $(LIB_BUILD_DIR)/$(subst .cpp,.oT,$(1))
C_SOURCE_TO_TEST_OBJECT = $(LIB_BUILD_DIR)/$(subst .c,.oT,$(1))

TEST_OBJECTS := $(foreach src,$(TEST_CPP_SRC_FILES),$(call CPP_SOURCE_TO_TEST_OBJECT,$(src)))
TEST_CXXFLAGS := $(CXXFLAGS) -DENABLE_TEST=1

define CREATE_CPP_TEST_OBJECT
$(call CPP_SOURCE_TO_TEST_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@install -d $$(dir $$@)
	@$(CXX) $(TEST_CXXFLAGS) -o $$@ -c $$^
endef

$(foreach src,$(TEST_CPP_SRC_FILES),$(eval $(call CREATE_CPP_TEST_OBJECT,$(src))))

print_test_vars :
	@echo TEST_CXXFLAGS=$(TEST_CXXFLAGS)
	@echo TEST_OBJECTS=$(TEST_OBJECTS)
	@echo TEST_LDFLAGS=$(TEST_LDFLAGS)

$(LIB_BUILD_DIR)/test: $(TEST_OBJECTS)
	@echo "\033[35mBUILD test\033[0m"
	@$(CXX) -o $@ $(TEST_OBJECTS) $(TEST_LDFLAGS) -lgmock_main -lgmock -lgtest -lpthread

#test : print_test_vars $(LIB_BUILD_DIR)/test
test : $(LIB_BUILD_DIR)/test

################################################################
# install and uninstall
################################################################

# install head files
SRC_HEAD_TO_INSTALL_HEAD = $(addprefix $(STAGING_DIR)/include/tbox/$(LIB_NAME)/,$(1))

define CREATE_INSTALL_HEAD_TARGET
$(call SRC_HEAD_TO_INSTALL_HEAD,$(1)) : $(1)
	@install -Dm 640 $$^ $$@
endef

$(foreach src,$(HEAD_FILES),$(eval $(call CREATE_INSTALL_HEAD_TARGET,$(src))))

INSTALL_HEADS := $(foreach src,$(HEAD_FILES),$(call SRC_HEAD_TO_INSTALL_HEAD,$(src)))

# install static library
ifeq ($(ENABLE_STATIC_LIB),yes)
INSTALL_STATIC_LIB := $(STAGING_DIR)/lib/$(STATIC_LIB)
$(INSTALL_STATIC_LIB) : $(LIB_BUILD_DIR)/$(STATIC_LIB)
	@install -Dm 640 $^ $@
endif

# install shared library
ifeq ($(ENABLE_SHARED_LIB),yes)
INSTALL_SHARED_LIB := $(INSTALL_DIR)/lib/$(SHARED_LIB)
$(INSTALL_SHARED_LIB) : $(LIB_BUILD_DIR)/$(SHARED_LIB)
	@install -d $(dir $@)
	@cd $(dir $@); \
	rm -f $(LIB_BASENAME).so*; \
	install -Dm 750 $^ $@; \
	ln -s $(SHARED_LIB) $(LIB_BASENAME).so; \
	ln -s $(SHARED_LIB) $(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y)
endif

install: $(INSTALL_HEADS) $(INSTALL_STATIC_LIB) $(INSTALL_SHARED_LIB)

uninstall:
	rm -rf $(STAGING_DIR)/include/tbox/$(LIB_NAME)
	rm -f $(STAGING_DIR)/lib/$(LIB_BASENAME).a
	rm -f $(INSTALL_DIR)/lib/$(LIB_BASENAME).so*

################################################################
# clean and distclean
################################################################
clean:
	-rm -f $(OBJECTS) $(SHARED_OBJECTS) $(TEST_OBJECTS)

