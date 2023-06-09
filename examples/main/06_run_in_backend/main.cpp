#include <tbox/main/main.h>
#include <iostream>
#include <thread>
#include <signal.h>

bool keep_running = true;

void SignalProc(int signo) {
  keep_running = false;
}

void MainLoop() {
  signal(SIGINT, SignalProc);
  signal(SIGTERM, SignalProc);

  while (keep_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::cout << '.' << std::flush;
  }
}

int main(int argc, char **argv) {
  if (!tbox::main::Start(argc, argv))
    return 0;

  MainLoop();
  tbox::main::Stop();

  return 0;
}
