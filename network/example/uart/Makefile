include ../build_env.mk

CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_network -ltbox_eventx -ltbox_event-ne -ltbox_base -levent_core -lev -lpthread

all : uart_tool uart_to_uart

uart_tool: uart_tool.o
	$(CXX) -o $@ $^ $(LDFLAGS)

uart_to_uart : uart_to_uart.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o

distclean: clean
	rm -f uart_tool uart_to_uart
