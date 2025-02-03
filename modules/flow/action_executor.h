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
#ifndef TBOX_FLOW_EXECUTOR_H_20221112
#define TBOX_FLOW_EXECUTOR_H_20221112

#include <deque>
#include <array>
#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/util/variables.h>

namespace tbox {
namespace flow {

class Action;

/**
 * 动作执行器
 *
 * FIXME: 没有做对Action::block()功能的支持
 */
class ActionExecutor {
  public:
    ActionExecutor();
    virtual ~ActionExecutor();

    NONCOPYABLE(ActionExecutor);
    IMMOVABLE(ActionExecutor);

  public:
    using ActionId = int;
    using ActionCallback = std::function<void(ActionId)>;
    using Callback = std::function<void()>;

    /**
     * \brief   追加一个要执行的动作
     *
     * \param action  动作对象
     * \param prio    优先级，0:紧急,1:普通,2:低，默认为普通
     *
     * \return  ActionId  动作ID
     */
    ActionId append(Action *action, int prio = 1);

    //! 获取当前正在执行的动作的ID
    ActionId current() const;

    //! 取消当前的动作
    bool cancelCurrent();

    /**
     * \brief   取消指定的动作
     * \param action_id   动作ID
     *
     * \return  true  如果找到该动作并删除成功
     * \return  false 没有找到该动作
     */
    bool cancel(ActionId action_id);

    //! 取消所有动作
    void cancelAll();

    //! set callbacks
    void setActionStartedCallback(const ActionCallback &cb) { action_started_cb_ = cb; }
    void setActionFinishedCallback(const ActionCallback &cb) { action_finished_cb_ = cb; }
    void setAllFinishedCallback(const Callback &cb) { all_finished_cb_ = cb; }

    inline util::Variables& vars() { return vars_; }

  private:
    ActionId allocActionId();
    void schedule();

  private:
    struct Item {
      ActionId id;
      Action *action;
    };

    std::array<std::deque<Item>, 3> action_deque_array_;
    ActionId action_id_alloc_counter_ = 0;
    int curr_action_deque_index_ = -1;  //!< 当前正在运行中的动作所在的队列，-1表示没有动作

    ActionCallback  action_started_cb_;
    ActionCallback  action_finished_cb_;
    Callback        all_finished_cb_;

    int cb_level_ = 0;

    util::Variables vars_;
};

}
}

#endif //TBOX_FLOW_EXECUTOR_H_20221112
