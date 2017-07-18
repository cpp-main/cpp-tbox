.PHONY: all clean distclean install uninstall

LIB_BASENAME=lib$(LIB_NAME)$(LIB_NAME_EXT)

STATIC_LIB := $(LIB_BASENAME).a
SHARED_LIB := $(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y).$(LIB_VERSION_Z)

all: install

################################################################
# static library
################################################################
STATIC_OBJECTS := $(subst .cpp,.o,$(SRC_FILES))
STATIC_CXXFLAGS := $(CCXXFLAGS)

$(STATIC_LIB) : $(STATIC_OBJECTS)
	$(AR) rc $@ $(STATIC_OBJECTS)

################################################################
# shared library
################################################################
SHARED_OBJECTS := $(subst .cpp,.oS,$(SRC_FILES))
SHARED_CXXFLAGS := $(CCXXFLAGS) -fPIC

$(SHARED_LIB) : $(SHARED_OBJECTS)
	-rm -f $(LIB_BASENAME).so*
	$(CXX) -shared $(SHARED_OBJECTS) -Wl,-soname,$(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y) -o $@
	-ln -s $@ $(LIB_BASENAME).so
	-ln -s $@ $(LIB_BASENAME).so.$(LIB_VERSION_X).$(LIB_VERSION_Y)

################################################################
# test
################################################################
TEST_OBJECTS := $(subst .cpp,.oT,$(SRC_FILES)) $(APPS_DIR)/base/util/log_output.oT
TEST_CXXFLAGS := $(CCXXFLAGS) -DENABLE_TEST

$(TEST_OBJECTS) : %.oT:%.cpp
	$(CXX) $(TEST_CXXFLAGS) -o $@ -c $^

test: $(TEST_OBJECTS)
	$(CXX) -o $@ $(TEST_OBJECTS) $(TEST_LDFLAGS) -lgtest -lpthread

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
install: $(STATIC_LIB)
	mkdir -p $(STAGING_DIR)/include/tbox/$(LIB_NAME)
	cp -dr $(HEAD_FILES) $(STAGING_DIR)/include/tbox/$(LIB_NAME)
	cp $(STATIC_LIB) $(STAGING_DIR)/lib/
	cp -d $(LIB_BASENAME).so* $(STAGING_DIR)/lib/

uninstall:
	rm -rf $(STAGING_DIR)/include/$(LIB_NAME)
	rm -f $(STAGING_DIR)/lib/$(LIB_BASENAME).*
