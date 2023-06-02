#include "app.h"
#include <base/log.h>

App::App(tbox::main::Context &ctx) :
  Module("app", ctx),
  timer_(ctx.loop()->newTimerEvent())
{
  LogTag();
}

App::~App() {
  LogTag();
  delete timer_;
}

bool App::onInit(const tbox::Json &cfg) {
  LogTag();
  timer_->initialize(std::chrono::seconds(1), tbox::event::Event::Mode::kPersist);
  timer_->setCallback(
    [] {
      LogInfo("timer tick");
    }
  );
  return true;
}

bool App::onStart() {
  LogTag();
  timer_->enable();
  return true;
}

void App::onStop() {
  LogTag();
  timer_->disable();
}

void App::onCleanup() {
  LogTag();
}
