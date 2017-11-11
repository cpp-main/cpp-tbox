include ../build_env.mk

CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_network -ltbox_eventx -ltbox_event-ne -ltbox_base -levent_core -lev -lpthread

all : send_only recv_only request respond 

send_only : send_only.o
	$(CXX) -o $@ $^ $(LDFLAGS)

recv_only : recv_only.o
	$(CXX) -o $@ $^ $(LDFLAGS)

request : request.o
	$(CXX) -o $@ $^ $(LDFLAGS)

respond : respond.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o

distclean: clean
	rm -f send_only
