.PHONY: test

all: install

install:
	cp -r include/tbox $(STAGING_DIR)/include/

uninstall:
	rm -rf $(STAGING_DIR)/include/tbox

CCFLAGS += -ggdb -I. -Wall -DENABLE_TEST -DLOG_MODULE_ID='"test"' -DBUILD_LOG_LEVEL=4
CXXFLAGS += $(CCFLAGS)
LDFLAGS += \
	-lgtest_main -lgtest \
	-lpthread

SRC_FILES= \
	util/log_output \
	util/test

TEST_OBJECTS=$(addsuffix .oT,$(SRC_FILES))

test : $(TEST_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TEST_OBJECTS) : %.oT:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $^

clean:
	find \( -name '*.o' -or -name '*.oT' \) -exec rm -f {} \;

distclean: clean
	-rm -f test
