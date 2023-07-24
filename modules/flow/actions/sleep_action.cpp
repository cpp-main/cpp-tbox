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
#include "sleep_action.h"

#include <tbox/base/assert.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace flow {

SleepAction::SleepAction(event::Loop &loop, const std::chrono::milliseconds &time_span) :
  Action(loop, "Sleep"),
  timer_(loop.newTimerEvent("SleepAction::timer_")),
  time_span_(time_span)
{
  TBOX_ASSERT(timer_ != nullptr);
  timer_->setCallback([this] { finish(true); });
}

SleepAction::SleepAction(event::Loop &loop, const Generator &gen) :
  Action(loop, "Sleep"),
  timer_(loop.newTimerEvent("SleepAction::timer_")),
  gen_(gen)
{
  TBOX_ASSERT(timer_ != nullptr);
  timer_->setCallback([this] { finish(true); });
}

SleepAction::~SleepAction() {
  delete timer_;
}

bool SleepAction::onStart() {
  auto time_span = gen_ ? gen_() : time_span_;

  //! 计算出到期时间点并保存到 finish_time_
  auto now = std::chrono::steady_clock::now();
  finish_time_ = now + time_span;

  timer_->initialize(time_span, event::Event::Mode::kOneshot);
  return timer_->enable();
}

bool SleepAction::onStop() {
  return timer_->disable();
}

bool SleepAction::onPause() {
  //! 计算剩余时长，并保存到 remain_time_span_ 中
  auto now = std::chrono::steady_clock::now();
  remain_time_span_ = std::chrono::duration_cast<std::chrono::milliseconds>(finish_time_ - now);

  return timer_->disable();
}

bool SleepAction::onResume() {
  //! 恢复 remain_time_span_
  timer_->initialize(remain_time_span_, event::Event::Mode::kOneshot);
  return timer_->enable();
}

void SleepAction::onReset() {
  timer_->disable();
}

}
}
