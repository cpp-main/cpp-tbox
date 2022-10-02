#include "action.h"

#include <cassert>

#include <tbox/base/log.h>
#include <tbox/base/json.hpp>

#include "context.h"

namespace tbox {
namespace action {

Action::Action(Context &ctx) :
  ctx_(ctx)
{}

Action::~Action() {
  assert(status_ == Status::kIdle);
}

void Action::toJson(Json &js) const {
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

  LogDbg("task %s start", type().c_str());
  ctx_.event_publisher().subscribe(this);
  status_ = Status::kRunning;
  return true;
}

bool Action::pause() {
  if (status_ != Status::kRunning) {
    LogWarn("not allow");
    return false;
  }

  LogDbg("task %s pause", type().c_str());
  ctx_.event_publisher().unsubscribe(this);
  status_ = Status::kPause;
  return true;
}

bool Action::resume() {
  if (status_ != Status::kPause) {
    LogWarn("not allow");
    return false;
  }

  LogDbg("task %s resume", type().c_str());
  ctx_.event_publisher().subscribe(this);
  status_ = Status::kRunning;
  return true;
}

bool Action::stop() {
  if (status_ == Status::kIdle)
    return false;

  LogDbg("task %s stop", type().c_str());
  ctx_.event_publisher().unsubscribe(this);
  status_ = Status::kIdle;
  return true;
}

bool Action::finish(bool is_done) {
  if (status_ == Status::kRunning ||
      status_ == Status::kPause) {
    LogDbg("task %s finish, is_done: %s", type().c_str(), is_done ? "yes" : "no");
    status_ = is_done ? Status::kDone : Status::kFail; 
    ctx_.loop().runInLoop(std::bind(finish_cb_, is_done));
    return true;

  } else {
    LogWarn("not allow");
    return false;
  }
}

std::string Action::ToString(Status status) {
  const char *tbl[] = { "idle", "running", "pause", "done", "fail" };
  return tbl[static_cast<size_t>(status)];
}

}
}
