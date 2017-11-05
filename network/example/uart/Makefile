include ../build_env.mk

CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_network -ltbox_eventx -ltbox_event-ne -ltbox_base -levent_core -lev -lpthread

all : uart_tool

uart_tool: uart_tool.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o
