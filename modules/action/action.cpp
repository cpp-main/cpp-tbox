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
  assert(status_ != Status::kRunning &&
         status_ != Status::kPause);
}

void Action::toJson(Json &js) const {
  js["id"] = id_;
  js["type"] = type();
  js["status"] = ToString(status_);
}

bool Action::start() {
  if (status_ != Status::kIdle) {
    LogWarn("not allow");
    return false;
  }

  if (!finish_cb_) {
    LogWarn("finish_cb not set");
    return false;
  }

  if (!onStart())
    return false;

  LogDbg("task %s|%s start", type().c_str(), id_.c_str());

  status_ = Status::kRunning;
  result_ = Result::kUnsure;
  return true;
}

bool Action::pause() {
  if (status_ != Status::kRunning) {
    LogWarn("not allow");
    return false;
  }

  if (!onPause())
    return false;

  LogDbg("task %s|%s pause", type().c_str(), id_.c_str());
  status_ = Status::kPause;
  return true;
}

bool Action::resume() {
  if (status_ != Status::kPause) {
    LogWarn("not allow");
    return false;
  }

  if (!onResume())
    return false;

  LogDbg("task %s|%s resume", type().c_str(), id_.c_str());
  status_ = Status::kRunning;
  return true;
}

bool Action::stop() {
  if (status_ == Status::kIdle)
    return false;

  if (!onStop())
    return false;

  LogDbg("task %s|%s stop", type().c_str(), id_.c_str());
  status_ = Status::kIdle;
  return true;
}

bool Action::finish(bool is_succ) {
  if (status_ != Status::kFinished) {
    LogDbg("task %s|%s finish, is_succ: %s", type().c_str(), id_.c_str(), is_succ? "succ" : "fail");
    status_ = Status::kFinished;
    result_ = is_succ ? Result::kSuccess : Result::kFail;
    loop_.runInLoop(std::bind(finish_cb_, is_succ));
    return true;

  } else {
    LogWarn("not allow");
    return false;
  }
}

std::string Action::ToString(Status status) {
  const char *tbl[] = { "idle", "running", "pause", "finished" };
  return tbl[static_cast<size_t>(status)];
}

}
}
