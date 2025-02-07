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

#include <sstream>

#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace flow {

int Action::_id_alloc_counter_ = 0;

Action::Action(event::Loop &loop, const std::string &type)
  : loop_(loop)
  , id_(++_id_alloc_counter_)
  , type_(type)
{}

Action::~Action() {
  if (isUnderway()) {
    LogWarn("action %d:%s[%s] is still underway, state:%s",
            id_, type_.c_str(), label_.c_str(),
            ToString(state_).c_str());
  }

  cancelDispatchedCallback();

  CHECK_DELETE_RESET_OBJ(timer_ev_);
}

void Action::toJson(Json &js) const {
  js["id"] = id_;
  js["type"] = type_;
  if (!label_.empty())
    js["label"] = label_;
  js["state"] = ToString(state_);
  js["result"] = ToString(result_);

  if (!vars_.empty())
    vars_.toJson(js["vars"]);
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

  is_base_func_invoked_ = false;

  onStart();

  if (!is_base_func_invoked_)
    LogWarn("%d:%s[%s] didn't invoke base func", id_, type_.c_str(), label_.c_str());

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

  is_base_func_invoked_ = false;

  onPause();

  if (!is_base_func_invoked_)
    LogWarn("%d:%s[%s] didn't invoke base func", id_, type_.c_str(), label_.c_str());

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

  is_base_func_invoked_ = false;

  onResume();

  if (!is_base_func_invoked_)
    LogWarn("%d:%s[%s] didn't invoke base func", id_, type_.c_str(), label_.c_str());

  if (last_state == state_) {
    if (timer_ev_ != nullptr)
      timer_ev_->enable();

    state_ = State::kRunning;
  }
  return true;
}

bool Action::stop() {
  if (!isUnderway())
    return true;

  LogDbg("stop action %d:%s[%s]", id_, type_.c_str(), label_.c_str());

  state_ = State::kStoped;

  if (timer_ev_ != nullptr)
    timer_ev_->disable();

  is_base_func_invoked_ = false;

  onStop();

  if (!is_base_func_invoked_)
    LogWarn("%d:%s[%s] didn't invoke base func", id_, type_.c_str(), label_.c_str());

  onFinal();

  return true;
}

bool Action::finish(bool is_succ, const Reason &why, const Trace &trace) {
  if (state_ != State::kFinished && state_ != State::kStoped) {
    LogDbg("action %d:%s[%s] finished, is_succ: %s", id_, type_.c_str(), label_.c_str(),
           (is_succ? "succ" : "fail"));

    state_ = State::kFinished;

    if (timer_ev_ != nullptr)
      timer_ev_->disable();

    is_base_func_invoked_ = false;

    onFinished(is_succ, why, trace);

    if (!is_base_func_invoked_)
      LogWarn("%d:%s[%s] didn't invoke base func", id_, type_.c_str(), label_.c_str());

    onFinal();
    return true;

  } else {
    LogWarn("not allow %d:%s[%s]", id_, type_.c_str(), label_.c_str());
    return false;
  }
}

bool Action::block(const Reason &why, const Trace &trace) {
  if (state_ != State::kFinished && state_ != State::kStoped) {
    LogDbg("action %d:%s[%s] blocked", id_, type_.c_str(), label_.c_str());

    state_ = State::kPause;

    is_base_func_invoked_ = false;

    onBlock(why, trace);

    if (!is_base_func_invoked_)
      LogWarn("%d:%s[%s] didn't invoke base func", id_, type_.c_str(), label_.c_str());

    return true;

  } else {
    LogWarn("not allow %d:%s[%s]", id_, type_.c_str(), label_.c_str());
    return false;
  }
}

void Action::reset() {
  if (state_ == State::kIdle)
    return;

  LogDbg("reset action %d:%s[%s]", id_, type_.c_str(), label_.c_str());

  if (isUnderway()) {
    LogWarn("be careful, action %d:%s[%s] in state:%s",
            id_, type_.c_str(), label_.c_str(), ToString(state_).c_str());
  }

  is_base_func_invoked_ = false;

  onReset();

  if (!is_base_func_invoked_)
    LogWarn("%d:%s[%s] didn't invoke base func", id_, type_.c_str(), label_.c_str());

  if (timer_ev_ != nullptr)
    timer_ev_->disable();

  cancelDispatchedCallback();

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

void Action::onStart() { is_base_func_invoked_ = true; }

void Action::onPause() { is_base_func_invoked_ = true; }

void Action::onResume() { is_base_func_invoked_ = true; }

void Action::onStop() { is_base_func_invoked_ = true; }

void Action::onReset() { is_base_func_invoked_ = true; }

void Action::onBlock(const Reason &why, const Trace &trace) {
  if (block_cb_) {
    Trace new_trace(trace);
    new_trace.emplace_back(id_, type_, label_);
    block_cb_run_id_ = loop_.runNext(std::bind(block_cb_, why, new_trace),
                                     std::string("Action::block") + ToString(new_trace));
  }

  is_base_func_invoked_ = true;
}

void Action::onFinished(bool is_succ, const Reason &why, const Trace &trace) {
  result_ = is_succ ? Result::kSuccess : Result::kFail;

  if (finish_cb_) {
    Trace new_trace(trace);
    new_trace.emplace_back(id_, type_, label_);
    finish_cb_run_id_ = loop_.runNext(std::bind(finish_cb_, is_succ, why, new_trace),
                                      std::string("Action::finish") + ToString(new_trace));
  }

  is_base_func_invoked_ = true;
}

void Action::onTimeout() {
  finish(false, Reason(ACTION_REASON_ACTION_TIMEOUT, "ActionTimeout"));
}

void Action::cancelDispatchedCallback() {
  if (finish_cb_run_id_ != 0) {
    loop_.cancel(finish_cb_run_id_);
    finish_cb_run_id_ = 0;
  }

  if (block_cb_run_id_ != 0) {
    loop_.cancel(block_cb_run_id_);
    block_cb_run_id_ = 0;
  }
}

bool Action::setParent(Action *parent) {
  //! 如果之前设置过了，就不能再设置
  if (parent != nullptr && parent_ != nullptr) {
    LogWarn("%d:%s[%s] can't set %d:%s[%s] as parent. its origin parent is %d:%s[%s]",
            id_, type_.c_str(), label_.c_str(),
            parent->id_, parent->type_.c_str(), parent->label_.c_str(),
            parent_->id_, parent_->type_.c_str(), parent_->label_.c_str());
    return false;
  }

  vars_.setParent(&(parent->vars_));
  parent_ = parent;
  return true;
}

Action::Reason::Reason(const Reason &other)
  : code(other.code)
  , message(other.message)
{ }

Action::Reason& Action::Reason::operator = (const Reason &other) {
  if (this != &other) {
    code = other.code;
    message = other.message;
  }
  return *this;
}

Action::Who::Who(const Who &other)
  : id(other.id)
  , type(other.type)
  , label(other.label)
{ }

Action::Who& Action::Who::operator = (const Who &other) {
  if (this != &other) {
    id = other.id;
    type = other.type;
    label = other.label;
  }
  return *this;
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

std::string ToString(const Action::Reason &reason) {
  std::stringstream oss;
  oss << '(' << reason.code << ':' << reason.message << ')';
  return oss.str();
}

std::string ToString(const Action::Trace &trace) {
  std::stringstream oss;
  bool is_first = true;

  for (const auto &who: trace) {
    if (!is_first)
      oss << "->";

    is_first = false;
    oss << '(' << who.id << ':' << who.type;
    if (!who.label.empty())
      oss << ',' << who.label;
    oss << ')';
  }

  return oss.str();
}

}
}
