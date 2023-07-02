MODULE=nc_client

TARGET:=lib$(MODULE).so

SHARED_OBJECTS:=nc_client.o

CXXFLAGS:=-I$(HOME)/.tbox/include -DLOG_MODULE_ID='"$(MODULE)"'
LDFLAGS:=-L$(HOME)/.tbox/lib -ltbox_network

$(TARGET): $(SHARED_OBJECTS)
	$(CXX) -shared $(SHARED_OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm *.o

distclean: clean
	rm $(TARGET)
