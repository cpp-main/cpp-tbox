include ../build_env.mk

TARGET := sub pub conn
CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_mqtt -ltbox_event -ltbox_base -lmosquitto -lpthread

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan

all : $(TARGET)

sub : sub.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

pub : pub.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

conn : conn.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf $(TARGET)
