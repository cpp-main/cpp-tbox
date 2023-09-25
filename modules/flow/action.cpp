/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "action.h"

#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace flow {

int Action::_id_alloc_counter_ = 0;

Action::Action(event::Loop &loop, const std::string &type) :
  loop_(loop),
  id_(++_id_alloc_counter_),
  type_(type)
{}

Action::~Action() {
  if (isUnderway()) {
    LogWarn("action %d:%s[%s] is still underway, state:%s",
            id_, type_.c_str(), label_.c_str(),
            ToString(state_).c_str());
  }

  if (finish_cb_run_id_ != 0) {
    loop_.cancel(finish_cb_run_id_);
    finish_cb_run_id_ = 0;
  }

  CHECK_DELETE_RESET_OBJ(timer_ev_);
}

void Action::toJson(Json &js) const {
  js["id"] = id_;
  js["type"] = type_;
  if (!label_.empty())
    js["label"] = label_;
  js["state"] = ToString(state_);
  js["result"] = ToString(result_);
}

bool Action::start() {
  if (state_ == State::kRunning)
    return true;

  if (state_ != State::kIdle) {
    LogWarn("not allow %d:%s[%s]", id_, type_.c_str(), label_.c_str());
    return false;
  }

  auto last_state = state_;

  LogDbg("start action %d:%s[%s]", id_, type_.c_str(), label_.c_str());
  if (!onStart()) {
    LogWarn("start action %d:%s[%s] fail", id_, type_.c_str(), label_.c_str());
    return false;
  }

  if (last_state == state_) {
    if (timer_ev_ != nullptr)
      timer_ev_->enable();

    state_ = State::kRunning;
  }
  return true;
}

bool Action::pause() {
  if (state_ == State::kPause)
    return true;

  if (state_ != State::kRunning) {
    LogWarn("not allow %d:%s[%s]", id_, type_.c_str(), label_.c_str());
    return false;
  }

  auto last_state = state_;

  LogDbg("pause action %d:%s[%s]", id_, type_.c_str(), label_.c_str());
  if (!onPause()) {
    LogWarn("pause action %d:%s[%s] fail", id_, type_.c_str(), label_.c_str());
    return false;
  }

  if (last_state == state_) {
    if (timer_ev_ != nullptr)
      timer_ev_->disable();

    state_ = State::kPause;
  }
  return true;
}

bool Action::resume() {
  if (state_ == State::kRunning)
    return true;

  if (state_ != State::kPause) {
    LogWarn("not allow %d:%s[%s]", id_, type_.c_str(), label_.c_str());
    return false;
  }

  auto last_state = state_;

  LogDbg("resume action %d:%s[%s]", id_, type_.c_str(), label_.c_str());
  if (!onResume()) {
    LogWarn("resume action %d:%s[%s] fail", id_, type_.c_str(), label_.c_str());
    return false;
  }

  if (last_state == state_) {
    if (timer_ev_ != nullptr)
      timer_ev_->enable();

    state_ = State::kRunning;
  }
  return true;
}

bool Action::stop() {
  if (state_ != State::kRunning &&
      state_ != State::kPause)
    return true;

  auto last_state = state_;

  LogDbg("stop action %d:%s[%s]", id_, type_.c_str(), label_.c_str());
  if (!onStop()) {
    LogWarn("stop action %d:%s[%s] fail", id_, type_.c_str(), label_.c_str());
    return false;
  }

  if (last_state == state_) {
    if (timer_ev_ != nullptr)
      timer_ev_->disable();

    state_ = State::kStoped;
  }
  return true;
}

void Action::reset() {
  if (state_ == State::kIdle)
    return;

  LogDbg("reset action %d:%s[%s]", id_, type_.c_str(), label_.c_str());
  onReset();

  if (timer_ev_ != nullptr)
    timer_ev_->disable();

  if (finish_cb_run_id_ != 0) {
    loop_.cancel(finish_cb_run_id_);
    finish_cb_run_id_ = 0;
  }

  state_ = State::kIdle;
  result_ = Result::kUnsure;
}

void Action::setTimeout(std::chrono::milliseconds ms) {
  LogDbg("set action %d:%s[%s] timeout: %d", id_, type_.c_str(), label_.c_str(), ms.count());

  if (timer_ev_ == nullptr) {
    timer_ev_ = loop_.newTimerEvent("Action::timer_ev_");
    timer_ev_->setCallback(
      [this] {
        LogDbg("action %d:%s[%s] timeout", id_, type_.c_str(), label_.c_str());
        onTimeout();
      }
    );
  } else {
    timer_ev_->disable();
  }

  timer_ev_->initialize(ms, event::Event::Mode::kOneshot);
  if (state_ == State::kRunning)
    timer_ev_->enable();
}

void Action::resetTimeout() {
  LogDbg("reset action %d:%s[%s] timeout", id_, type_.c_str(), label_.c_str());

  if (timer_ev_ != nullptr) {
    auto tobe_delete = timer_ev_;
    timer_ev_ = nullptr;

    tobe_delete->disable();
    loop_.runNext([tobe_delete] { delete tobe_delete; }, "Action::resetTimeout, delete");
  }
}

bool Action::finish(bool is_succ) {
  if (state_ != State::kFinished && state_ != State::kStoped) {
    LogDbg("action %d:%s[%s] finished, is_succ: %s", id_, type_.c_str(), label_.c_str(),
           (is_succ? "succ" : "fail"));
    state_ = State::kFinished;

    if (timer_ev_ != nullptr)
      timer_ev_->disable();

    result_ = is_succ ? Result::kSuccess : Result::kFail;

    if (finish_cb_)
        finish_cb_run_id_ = loop_.runNext(std::bind(finish_cb_, is_succ), "Action::finish");
    else
        LogWarn("action %d:%s[%s] no finish_cb", id_, type_.c_str(), label_.c_str());

    onFinished(is_succ);
    return true;

  } else {
    LogWarn("not allow %d:%s[%s]", id_, type_.c_str(), label_.c_str());
    return false;
  }
}

std::string ToString(Action::State state) {
  const char *tbl[] = { "idle", "running", "pause", "finished", "stoped" };
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
