include ../build_env.mk

TARGET := calc_game
OBJECTS := main.o game.o game_lite.o
LDFLAGS += -L.. -ltbox_event -ltbox_base -levent_core -lev

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJECTS)

distclean: clean
	rm -f $(TARGET)
