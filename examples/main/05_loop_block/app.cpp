#include "app.h"
#include <thread>
#include <tbox/base/log.h>

App::App(tbox::main::Context &ctx) :
    Module("app", ctx)
{ }

bool App::onStart() {
  timer_ = ctx().timer_pool()->doEvery(
    std::chrono::seconds(5),
    [] (tbox::eventx::TimerPool::TimerToken) {
      LogDbg("begin sleep 2s");
      std::this_thread::sleep_for(std::chrono::seconds(2));
      LogDbg("end");
    }
  );
  return true;
}

void App::onStop() {
  ctx().timer_pool()->cancel(timer_);
  timer_.reset();
}
