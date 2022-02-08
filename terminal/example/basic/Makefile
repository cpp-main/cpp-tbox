include ../build_env.mk

TARGET := demo
OBJECTS := main.o
CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_terminal -ltbox_util -ltbox_network -ltbox_event-ne -ltbox_base -levent_core -lev -lpthread

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJECTS)
