include ../build_env.mk

TARGET := calc_game
OBJECTS := main.o game.o game_lite.o
LDFLAGS += -L.. -ltbox_event-ne -ltbox_base -levent_core -lev

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJECTS)
