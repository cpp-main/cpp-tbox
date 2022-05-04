include ../build_env.mk

TARGET := server

CXXFLAGS += -ggdb -DLOG_MODULE_ID='"main"'
LDFLAGS += -ltbox_http -ltbox_network -ltbox_log -ltbox_event -ltbox_util -ltbox_base

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan

all : $(TARGET)

$(TARGET) : main.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o $(TARGET)
