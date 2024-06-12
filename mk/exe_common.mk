################################################################
# ABOUT:
# This C++ library project Makefile's common part, which can be included
# into project's Makefile, to make Makefile is easy to maintain.
#
# HOW TO USE:
# before include this file. those variables need be specified.
# PROJECT, EXE_NAME, C_SRC_FILES, CC_SRC_FILES, CPP_SRC_FILES, CXXFLAGS, TEST_LDFLAGS
# BUILD_DIR, INSTALL_DIR
#
# TARGETS:
# all, test, clean, distclean, install, uninstall
################################################################

.PHONY: all print_complie_vars print_build_var print_test_vars clean distclean install uninstall pre_build post_build

EXE_BUILD_DIR = $(BUILD_DIR)/$(PROJECT)

#TARGETS := pre_build print_vars build_exe post_build
TARGETS := pre_build build_exe post_build

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
# exe
################################################################

CPP_SOURCE_TO_OBJECT = $(EXE_BUILD_DIR)/$(subst .cpp,.o,$(1))
CC_SOURCE_TO_OBJECT = $(EXE_BUILD_DIR)/$(subst .cc,.o,$(1))
C_SOURCE_TO_OBJECT = $(EXE_BUILD_DIR)/$(subst .c,.o,$(1))

OBJECTS := $(foreach src,$(CPP_SRC_FILES),$(call CPP_SOURCE_TO_OBJECT,$(src)))
OBJECTS += $(foreach src,$(CC_SRC_FILES),$(call CC_SOURCE_TO_OBJECT,$(src)))
OBJECTS += $(foreach src,$(C_SRC_FILES),$(call C_SOURCE_TO_OBJECT,$(src)))

define CREATE_CPP_OBJECT
$(call CPP_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@install -d $$(dir $$@)
	@$(CXX) $(CXXFLAGS) -o $$@ -c $$^
endef

define CREATE_CC_OBJECT
$(call CC_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCXX $$^\033[0m"
	@install -d $$(dir $$@)
	@$(CXX) $(CXXFLAGS) -o $$@ -c $$^
endef

define CREATE_C_OBJECT
$(call C_SOURCE_TO_OBJECT,$(1)) : $(1)
	@echo "\033[32mCC $$^\033[0m"
	@install -d $$(dir $$@)
	@$(CC) $(CFLAGS) -o $$@ -c $$^
endef

$(foreach src,$(CPP_SRC_FILES),$(eval $(call CREATE_CPP_OBJECT,$(src))))
$(foreach src,$(CC_SRC_FILES),$(eval $(call CREATE_CC_OBJECT,$(src))))
$(foreach src,$(C_SRC_FILES),$(eval $(call CREATE_C_OBJECT,$(src))))

print_exe_vars :
	@echo CFLAGS=$(CFLAGS)
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo EXE_NAME=$(EXE_NAME)
	@echo OBJECTS=$(OBJECTS)

$(EXE_BUILD_DIR)/$(EXE_NAME) : $(OBJECTS)
	@echo "\033[35mBUILD $(EXE_NAME) \033[0m"
	@install -d $(dir $@)
	@$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) -rdynamic

#build_exe : print_exe_vars $(EXE_BUILD_DIR)/$(EXE_NAME)
build_exe : $(EXE_BUILD_DIR)/$(EXE_NAME)

################################################################
# test
################################################################

CPP_SOURCE_TO_TEST_OBJECT = $(EXE_BUILD_DIR)/$(subst .cpp,.oT,$(1))
C_SOURCE_TO_TEST_OBJECT = $(EXE_BUILD_DIR)/$(subst .c,.oT,$(1))

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

$(EXE_BUILD_DIR)/test: $(TEST_OBJECTS)
	@echo "\033[35mBUILD test\033[0m"
	@install -d $(dir $@)
	@$(CXX) -o $@ $(TEST_OBJECTS) $(TEST_LDFLAGS) -lgmock_main -lgmock -lgtest -lpthread

#test : print_test_vars $(EXE_BUILD_DIR)/test
test : $(EXE_BUILD_DIR)/test

################################################################
# install and uninstall
################################################################

# install head files
SRC_CONF_TO_INSTALL_CONF = $(addprefix $(INSTALL_DIR)/etc/$(EXE_NAME)/,$(1))

define CREATE_INSTALL_CONF_TARGET
$(call SRC_CONF_TO_INSTALL_CONF,$(1)) : $(1)
	@install -Dm 640 $$^ $$@
endef

$(foreach src,$(CONF_FILES),$(eval $(call CREATE_INSTALL_CONF_TARGET,$(src))))

INSTALL_CONFS := $(foreach src,$(CONF_FILES),$(call SRC_CONF_TO_INSTALL_CONF,$(src)))

INSTALL_EXE := $(INSTALL_DIR)/bin/$(EXE_NAME)
$(INSTALL_EXE) : $(EXE_BUILD_DIR)/$(EXE_NAME)
	@install -Dm 750 $^ $@

install: $(INSTALL_EXE) $(INSTALL_CONFS)

uninstall:
	rm -f $(INSTALL_DIR)/bin/$(EXE_NAME)

################################################################
# clean
################################################################
clean:
	-rm -f $(OBJECTS)

