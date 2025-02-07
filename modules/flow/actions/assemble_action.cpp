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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#include "assemble_action.h"
#include <tbox/base/log.h>

namespace tbox {
namespace flow {

int AssembleAction::addChild(Action *)
{
  LogWarn("%d:%s[%s] not implement this function", id(), type().c_str(), label().c_str());
  return -1;
}

int AssembleAction::addChildAs(Action *, const std::string &)
{
  LogWarn("%d:%s[%s] not implement this function", id(), type().c_str(), label().c_str());
  return -1;
}

bool AssembleAction::setChild(Action *)
{
  LogWarn("%d:%s[%s] not implement this function", id(), type().c_str(), label().c_str());
  return false;
}

bool AssembleAction::setChildAs(Action *, const std::string &)
{
  LogWarn("%d:%s[%s] not implement this function", id(), type().c_str(), label().c_str());
  return false;
}

void AssembleAction::onFinal()
{
    if (final_cb_)
        final_cb_();
}

//////////////////////////
// SerialAssembleAction
//////////////////////////

bool SerialAssembleAction::startThisAction(Action *action)
{
    if (action->start()) {
        curr_action_ = action;
        return true;
    }

    return false;
}

void SerialAssembleAction::stopCurrAction()
{
    if (curr_action_ != nullptr) {
        curr_action_->stop();
        curr_action_ = nullptr;
    }
}

bool SerialAssembleAction::handleChildFinishEvent(ChildFinishFunc &&child_finish_func)
{
    curr_action_ = nullptr;

    //! 如果处于运行状态，则由派生类自行处理
    if (state() == State::kRunning)
        return false;

    //! 如果处于暂停状态，则暂存结果
    if (state() == State::kPause)
        child_finish_func_ = std::move(child_finish_func);

    //! 其它状态，如已结束或停止，则不处理
    return true;
}

void SerialAssembleAction::onLastChildFinished(bool is_succ, const Reason &reason, const Trace &trace)
{
    curr_action_ = nullptr;

    //! 如果处于运行状态，则正常退出
    if (state() == State::kRunning) {
        finish(is_succ, reason, trace);
        return;
    }

    //! 如果处于暂停状态，则暂存结果
    if (state() == State::kPause)
        child_finish_func_ = [this, is_succ, reason, trace] { finish(is_succ, reason, trace); };

    //! 其它状态，如已结束或停止，则不处理
}

void SerialAssembleAction::onPause()
{
    if (curr_action_ != nullptr)
        curr_action_->pause();

    AssembleAction::onPause();
}

void SerialAssembleAction::onResume()
{
    AssembleAction::onResume();

    if (curr_action_ != nullptr) {
        curr_action_->resume();

    } else if (child_finish_func_) {
        loop_.runNext(std::move(child_finish_func_));

    } else {
        LogWarn("%d:%s[%s] can't resume", id(), type().c_str(), label().c_str());
    }
}

void SerialAssembleAction::onStop()
{
    stopCurrAction();
    child_finish_func_ = nullptr;

    AssembleAction::onStop();
}

void SerialAssembleAction::onReset()
{
    curr_action_ = nullptr;
    child_finish_func_ = nullptr;

    AssembleAction::onReset();
}

}
}
