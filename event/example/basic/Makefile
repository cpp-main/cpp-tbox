include ../build_env.mk

TARGETS=io_example timer_example signal_example \
		run_in_loop_demo stdin_timer_signal_demo \
		run_next_seq_demo delay_delete_demo

CXXFLAGS += -ggdb
#LDFLAGS += -ltbox_event -ltbox_base -levent_core -lev -lpthread
LDFLAGS += -ltbox_event -ltbox_base -lpthread

CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan

all: $(TARGETS)

stdin_timer_signal_demo : stdin_timer_signal_demo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

run_in_loop_demo : run_in_loop_demo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

run_next_seq_demo : run_next_seq_demo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

timer_example : timer_example.o
	$(CXX) -o $@ $^ $(LDFLAGS)

signal_example : signal_example.o
	$(CXX) -o $@ $^ $(LDFLAGS)

io_example : io_example.o
	$(CXX) -o $@ $^ $(LDFLAGS)

delay_delete_demo : delay_delete_demo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o

distclean : clean
	rm $(TARGETS)

install:

uninstall:
