include ../build_env.mk

TARGET := io_echo
OBJECTS := io_echo.o
CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. -ltbox_network -ltbox_eventx -ltbox_event-ne -ltbox_base -levent_core -lev -lpthread

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJECTS)

distclean: clean
	rm -f $(TARGET)
