#include "action.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace flow {

Action::Action(event::Loop &loop) :
  loop_(loop)
{}

Action::~Action() {
  //! 不允许在未stop之前或是未结束之前析构对象
  TBOX_ASSERT(state_ != State::kRunning && state_ != State::kPause);
  CHECK_DELETE_RESET_OBJ(timer_ev_);
}

void Action::toJson(Json &js) const {
  js["type"] = type();
  js["state"] = ToString(state_);
  js["result"] = ToString(result_);
  js["name"] = name_;
}

bool Action::start() {
  if (state_ == State::kRunning)
    return true;

  if (state_ != State::kIdle) {
    LogWarn("not allow");
    return false;
  }

  if (!finish_cb_) {
    LogWarn("finish_cb not set");
    return false;
  }

  LogDbg("start action %s(%s)", type().c_str(), name_.c_str());
  if (!onStart()) {
    LogWarn("start action %s(%s) fail", type().c_str(), name_.c_str());
    return false;
  }

  if (timer_ev_ != nullptr)
    timer_ev_->enable();

  state_ = State::kRunning;
  return true;
}

bool Action::pause() {
  if (state_ == State::kPause)
    return true;

  if (state_ != State::kRunning) {
    LogWarn("not allow");
    return false;
  }

  LogDbg("pause action %s(%s)", type().c_str(), name_.c_str());
  if (!onPause()) {
    LogWarn("pause action %s(%s) fail", type().c_str(), name_.c_str());
    return false;
  }

  if (timer_ev_ != nullptr)
    timer_ev_->disable();

  state_ = State::kPause;
  return true;
}

bool Action::resume() {
  if (state_ == State::kRunning)
    return true;

  if (state_ != State::kPause) {
    LogWarn("not allow");
    return false;
  }

  LogDbg("resume action %s(%s)", type().c_str(), name_.c_str());
  if (!onResume()) {
    LogWarn("resume action %s(%s) fail", type().c_str(), name_.c_str());
    return false;
  }

  if (timer_ev_ != nullptr)
    timer_ev_->enable();

  state_ = State::kRunning;
  return true;
}

bool Action::stop() {
  if (state_ != State::kRunning &&
      state_ != State::kPause)
    return true;

  LogDbg("stop action %s(%s)", type().c_str(), name_.c_str());
  if (!onStop()) {
    LogWarn("stop action %s(%s) fail", type().c_str(), name_.c_str());
    return false;
  }

  if (timer_ev_ != nullptr)
    timer_ev_->disable();

  state_ = State::kStoped;
  return true;
}

void Action::reset() {
  if (state_ == State::kIdle)
    return;

  LogDbg("reset action %s(%s)", type().c_str(), name_.c_str());
  onReset();

  if (timer_ev_ != nullptr)
    timer_ev_->disable();

  state_ = State::kIdle;
  result_ = Result::kUnsure;
}

void Action::setTimeout(std::chrono::milliseconds ms) {
  if (timer_ev_ == nullptr) {
    timer_ev_ = loop_.newTimerEvent();
    timer_ev_->setCallback(
      [this] {
        LogDbg("action %s(%s) timeout", type().c_str(), name_.c_str());
        onTimeout();
      }
    );
  } else {
    timer_ev_->disable();
  }
  timer_ev_->initialize(ms, event::Event::Mode::kOneshot);
}

bool Action::finish(bool is_succ) {
  if (state_ != State::kFinished) {
    LogDbg("action %s(%s) finished, is_succ: %s", type().c_str(),
           name_.c_str(), is_succ? "succ" : "fail");
    state_ = State::kFinished;

    if (timer_ev_ != nullptr)
      timer_ev_->disable();

    result_ = is_succ ? Result::kSuccess : Result::kFail;
    loop_.runInLoop(std::bind(finish_cb_, is_succ));
    onFinished(is_succ);
    return true;

  } else {
    LogWarn("not allow");
    return false;
  }
}

std::string ToString(Action::State state) {
  const char *tbl[] = { "idle", "running", "pause", "finished" };
  auto index = static_cast<size_t>(state);
  if (index < NUMBER_OF_ARRAY(tbl))
    return tbl[index];
  return "unknown";
}

std::string ToString(Action::Result result) {
  const char *tbl[] = { "unsure", "success", "fail" };
  auto index = static_cast<size_t>(result);
  if (index < NUMBER_OF_ARRAY(tbl))
    return tbl[index];
  return "unknown";
}

}
}
