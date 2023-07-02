MODULE=timer_event

TARGET:=lib$(MODULE).so

SHARED_OBJECTS:=timer_event.o

CXXFLAGS:=-I$(HOME)/.tbox/include -DLOG_MODULE_ID='"$(MODULE)"'
LDFLAGS:=-L$(HOME)/.tbox/lib

$(TARGET): $(SHARED_OBJECTS)
	$(CXX) -shared $(SHARED_OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm *.o

distclean: clean
	rm $(TARGET)
