include ../build_env.mk

CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_network -ltbox_eventx -ltbox_event-ne -ltbox_base -levent_core -lev -lpthread

all : io_echo_1 io_echo_2

io_echo_1 : io_echo_1.o
	$(CXX) -o $@ $^ $(LDFLAGS)

io_echo_2 : io_echo_2.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o
