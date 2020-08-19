include ../build_env.mk

TARGET := sub pub
CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_mqtt -ltbox_event-ne -ltbox_base -levent_core -lev -lmosquitto -lpthread

all : $(TARGET)

sub : sub.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

pub : pub.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf sub pub
