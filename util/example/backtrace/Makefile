include ../build_env.mk

CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_util

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan

all : sample

sample: sample.o
	$(CXX) -o $@ $^ $(LDFLAGS) -rdynamic

clean:
	rm -rf *.o

distclean: clean
	rm -f sample
