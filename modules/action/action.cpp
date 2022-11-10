#include "action.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>

namespace tbox {
namespace action {

Action::Action(event::Loop &loop, const std::string &id) :
  loop_(loop), id_(id)
{}

Action::~Action() {
  //! 不允许在未stop之前或是未结束之前析构对象
  assert(state_ != State::kRunning &&
         state_ != State::kPause);
}

void Action::toJson(Json &js) const {
  js["id"] = id_;
  js["type"] = type();
  js["state"] = ToString(state_);
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

  LogDbg("start action %s|%s", type().c_str(), id_.c_str());
  if (!onStart()) {
    LogWarn("start action %s|%s fail", type().c_str(), id_.c_str());
    return false;
  }

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

  LogDbg("pause action %s|%s", type().c_str(), id_.c_str());
  if (!onPause()) {
    LogWarn("pause action %s|%s fail", type().c_str(), id_.c_str());
    return false;
  }

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

  LogDbg("resume action %s|%s", type().c_str(), id_.c_str());
  if (!onResume()) {
    LogWarn("resume action %s|%s fail", type().c_str(), id_.c_str());
    return false;
  }

  state_ = State::kRunning;
  return true;
}

bool Action::stop() {
  if (state_ != State::kRunning &&
      state_ != State::kPause)
    return true;

  LogDbg("stop action %s|%s", type().c_str(), id_.c_str());
  if (!onStop()) {
    LogWarn("stop action %s|%s fail", type().c_str(), id_.c_str());
    return false;
  }

  state_ = State::kStoped;
  return true;
}

void Action::reset() {
  if (state_ == State::kIdle)
    return;

  LogDbg("reset action %s|%s", type().c_str(), id_.c_str());
  onReset();

  state_ = State::kIdle;
  result_ = Result::kUnsure;
}

bool Action::finish(bool is_succ) {
  if (state_ != State::kFinished) {
    LogDbg("action %s|%s finished, is_succ: %s",
        type().c_str(), id_.c_str(), is_succ? "succ" : "fail");
    state_ = State::kFinished;
    result_ = is_succ ? Result::kSuccess : Result::kFail;
    loop_.runInLoop(std::bind(finish_cb_, is_succ));
    onFinished(is_succ);
    return true;

  } else {
    LogWarn("not allow");
    return false;
  }
}

std::string Action::ToString(State state) {
  const char *tbl[] = { "idle", "running", "pause", "finished" };
  return tbl[static_cast<size_t>(state)];
}

}
}
