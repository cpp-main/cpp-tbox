include ../build_env.mk

TARGETS=io_example timer_example signal_example run_in_loop_demo stdin_timer_signal

CXXFLAGS += -ggdb
LDFLAGS += -levent -lpthread

all: $(TARGETS)

stdin_timer_signal : stdin_timer_signal.o log_output.o
	$(CXX) -o $@ $^ $(LDFLAGS)

run_in_loop_demo : run_in_loop_demo.o log_output.o
	$(CXX) -o $@ $^ $(LDFLAGS)

timer_example : timer_example.o log_output.o
	$(CXX) -o $@ $^ $(LDFLAGS)

signal_example : signal_example.o log_output.o
	$(CXX) -o $@ $^ $(LDFLAGS)

io_example : io_example.o log_output.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o
