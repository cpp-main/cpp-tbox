include ../build_env.mk

.PHONY : all clean distclean

TARGET := sample
CXXFLAGS += -ggdb -DLOG_MODULE_ID='"demo"'
LDFLAGS += -L.. \
	-ltbox_main \
	-ltbox_eventx \
	-ltbox_event-ne \
	-ltbox_util \
	-ltbox_base \
	-levent_core \
	-lev -lpthread

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan
OBJECTS := reg_apps.o app.o

all : $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf $(OBJECTS)

distclean: clean
	rm -rf $(TARGET)
