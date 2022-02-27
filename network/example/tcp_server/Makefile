include ../build_env.mk

CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_network -ltbox_eventx -ltbox_event -ltbox_base -levent_core -lev -lpthread
TARGETS = tcp_echo tcp_nc_server

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan

all : $(TARGETS)

tcp_echo : tcp_echo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

tcp_nc_server : tcp_nc_server.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o

distclean: clean
	rm -f $(TARGETS)
