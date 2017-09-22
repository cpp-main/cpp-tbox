include ../build_env.mk

TARGET := calc_game
OBJECTS := main.o game.o game_lite.o log_output.o
LDFLAGS += -L.. -ltbox_event-ne -levent_core

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJECTS)
