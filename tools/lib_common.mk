################################################################
# ABOUT:
# This C++ library project Makefile's common part, which can be included
# into project's Makefile, to make Makefile is easy to maintain.
#
# HOW TO USE:
# before include this file. those variables need be specified.
# LIB_VERSION_X, LIB_VERSION_Y, LIB_VERSION_Z, SRC_FILES, CCXXFLAGS, TEST_LDFLAGS
#
# TARGETS:
# all, libxxx.a, libxxx.so.x.x.x, test, clean, distclean, install, uninstall
################################################################

.PHONY: all print_vars print_static_vars print_shared_vars print_test_vars clean distclean install uninstall

LIB_BASENAME=libtbox_$(LIB_NAME)$(LIB_NAME_EXT)

STATIC_LIB := $(LIB_BASENAME).a
SHARED_LIB := $(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y).$(LIB_VERSION_Z)

ENABLE_STATIC_LIB ?= yes
ENABLE_SHARED_LIB ?= yes

TARGETS := print_vars

ifeq ($(ENABLE_STATIC_LIB),yes)
	TARGETS += $(STATIC_LIB)
endif

ifeq ($(ENABLE_SHARED_LIB),yes)
	TARGETS += $(SHARED_LIB)
endif

all: $(TARGETS) install

print_vars:
	@echo CXX=$(CXX)

################################################################
# static library
################################################################
STATIC_OBJECTS := $(subst .cpp,.o,$(SRC_FILES))

print_static_vars :
	@echo STATIC_LIB=$(STATIC_LIB)
	@echo CCXXFLAGS=$(CCXXFLAGS)

.SUFFIXES: .cpp .c

.cpp.o:
	@echo "\033[32mCXX $^\033[0m"
	@$(CXX) $(CCXXFLAGS) -o $@ -c $^
.c.o:
	@echo "\033[32mCXX $^\033[0m"
	@$(CC) $(CCXXFLAGS) -o $@ -c $^

$(STATIC_LIB) : print_static_vars $(STATIC_OBJECTS)
	@echo "\033[32mBUILD $@\033[0m"
	$(AR) rc $@ $(STATIC_OBJECTS)

################################################################
# shared library
################################################################
SHARED_OBJECTS := $(subst .cpp,.oS,$(SRC_FILES))
SHARED_CXXFLAGS := $(CCXXFLAGS) -fPIC

print_shared_vars :
	@echo SHARED_LIB=$(SHARED_LIB)
	@echo SHARED_CXXFLAGS=$(SHARED_CXXFLAGS)

$(SHARED_OBJECTS) : %.oS:%.cpp
	@echo "\033[32mCXX $^\033[0m"
	@$(CXX) $(SHARED_CXXFLAGS) -o $@ -c $^

$(SHARED_LIB) : print_shared_vars $(SHARED_OBJECTS)
	-rm -f $(LIB_BASENAME).so*
	@echo "\033[32mBUILD $@\033[0m"
	$(CXX) -shared $(SHARED_OBJECTS) -Wl,-soname,$(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y) -o $@
	-ln -s $@ $(LIB_BASENAME).so
	-ln -s $@ $(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y)

################################################################
# test
################################################################
TEST_SRC_FILES += $(SRC_FILES)
TEST_OBJECTS := $(subst .cpp,.o,$(TEST_SRC_FILES))

print_test_vars :
	@echo TEST_OBJECTS=$(TEST_OBJECTS)

test: print_test_vars $(TEST_OBJECTS)
	@echo "\033[32mBUILD $@\033[0m"
	$(CXX) -o $@ $(TEST_OBJECTS) $(TEST_LDFLAGS) -lgmock_main -lgmock -lgtest -lpthread

################################################################
# clean and distclean
################################################################
clean:
	-rm -f $(STATIC_OBJECTS) $(SHARED_OBJECTS) $(TEST_OBJECTS)

distclean: clean
	-rm -f $(LIB_BASENAME).* test

################################################################
# install and uninstall
################################################################
install:
	mkdir -p $(STAGING_DIR)/include/tbox/$(LIB_NAME)
	cp -dr $(HEAD_FILES) $(STAGING_DIR)/include/tbox/$(LIB_NAME)
ifeq ($(ENABLE_STATIC_LIB),yes)
	cp $(STATIC_LIB) $(STAGING_DIR)/lib/
endif
ifeq ($(ENABLE_SHARED_LIB),yes)
	cp -d $(LIB_BASENAME).so* $(STAGING_DIR)/lib/
endif

uninstall:
	rm -rf $(STAGING_DIR)/include/$(LIB_NAME)
	rm -f $(STAGING_DIR)/lib/$(LIB_BASENAME).*
