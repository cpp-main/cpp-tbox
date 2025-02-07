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
#include "action_executor.h"

#include <algorithm>
#include <tbox/base/assert.h>
#include "action.h"

namespace tbox {
namespace flow {

ActionExecutor::ActionExecutor() { }

ActionExecutor::~ActionExecutor() {
  TBOX_ASSERT(cb_level_ == 0);
  for (auto &action_deque : action_deque_array_) {
    for (auto item : action_deque)
          delete item.action;
      action_deque.clear();
  }
}

ActionExecutor::ActionId ActionExecutor::append(Action *action, int prio) {
  TBOX_ASSERT(0 <= prio && prio <= 2);
  TBOX_ASSERT(action != nullptr);

  Item item = {
    .id = allocActionId(),
    .action = action
  };
  action_deque_array_.at(prio).push_back(item);
  action->setFinishCallback([this](bool, const Action::Reason &, const Action::Trace &) { schedule(); });
  action->vars().setParent(&vars_);
  schedule();
  return item.id;
}

ActionExecutor::ActionId ActionExecutor::current() const {
  if (curr_action_deque_index_ == -1)
    return -1;

  auto &action_deque = action_deque_array_.at(curr_action_deque_index_);
  return action_deque.front().id;
}

bool ActionExecutor::cancelCurrent() {
  if (curr_action_deque_index_ == -1)
    return false;

  auto &action_deque = action_deque_array_.at(curr_action_deque_index_);
  auto item = action_deque.front();
  item.action->stop();
  delete item.action;
  action_deque.pop_front();
  schedule();
  return true;
}

bool ActionExecutor::cancel(ActionId action_id) {
  TBOX_ASSERT(action_id > 0);

  for (auto &action_deque : action_deque_array_) {
    auto iter = std::find_if(action_deque.begin(), action_deque.end(),
      [action_id] (const Item &item) { return item.id == action_id; }
    );

    if (iter != action_deque.end()) {
      delete iter->action;
      action_deque.erase(iter);
      schedule();
      return true;
    }
  }
  return false;
}

void ActionExecutor::cancelAll() {
  for (auto &action_deque : action_deque_array_) {
    if (!action_deque.empty()) {
      auto action_item = action_deque.front();
      action_item.action->stop();
    }
  }
}

ActionExecutor::ActionId ActionExecutor::allocActionId() { return ++action_id_alloc_counter_; }

void ActionExecutor::schedule() {
  TBOX_ASSERT(cb_level_ == 0);

  while (true) {
    //! 找出优先级最高，且不为空的队列
    int ready_deque_index = -1;
    for (int i = 0; i < 3; ++i) {
      if (!action_deque_array_.at(i).empty()) {
        ready_deque_index = i;
        break;
      }
    }

    //! 如果所有的队列都是空的，那么就直接退出
    if (ready_deque_index == -1) {
      ++cb_level_;
      if (all_finished_cb_)
        all_finished_cb_();
      --cb_level_;
      return;
    }

    //! 如果高优先级的队列比当前队列优先级还高，则要先暂停当前队列头部的动作
    if (curr_action_deque_index_ != -1 &&
        ready_deque_index < curr_action_deque_index_) {
      action_deque_array_.at(curr_action_deque_index_).front().action->pause();
    }

    auto &ready_deque = action_deque_array_.at(ready_deque_index);
    while (!ready_deque.empty()) {
      auto item = ready_deque.front();
      //! 没有启动的要启动
      if (item.action->state() == Action::State::kIdle) {
        if (item.action->start()) {
          curr_action_deque_index_ = ready_deque_index;
          ++cb_level_;
          if (action_started_cb_)
            action_started_cb_(item.id);
          --cb_level_;
        } else {
          ready_deque.pop_front();
          delete item.action;
          curr_action_deque_index_ = -1;
          ++cb_level_;
          if (action_finished_cb_)
            action_finished_cb_(item.id);
          --cb_level_;
        }

      //! 被暂停了的，要恢复
      } else if (item.action->state() == Action::State::kPause) {
        curr_action_deque_index_ = ready_deque_index;
        item.action->resume();

      //! 已完成了的，要删除
      } else if (item.action->state() == Action::State::kFinished ||
                 item.action->state() == Action::State::kStoped) {
        ready_deque.pop_front();
        delete item.action;
        curr_action_deque_index_ = -1;
        ++cb_level_;
        if (action_finished_cb_)
          action_finished_cb_(item.id);
        --cb_level_;

      //! 其它状态：运行中，不需要处理
      } else {
        return;
      }
    }
  }
}

}
}
