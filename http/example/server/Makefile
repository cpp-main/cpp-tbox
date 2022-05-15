include ../build_env.mk

TARGET := simple async_respond router

CXXFLAGS += -ggdb -DLOG_MODULE_ID='"main"'
LDFLAGS += -ltbox_http -ltbox_network -ltbox_log -ltbox_eventx -ltbox_event -ltbox_util -ltbox_base

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan

all : $(TARGET)

simple : simple.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

async_respond : async_respond.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

router: router.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o $(TARGET)
