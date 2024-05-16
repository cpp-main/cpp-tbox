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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_FLOW_STATE_MACHINE_H_20220320
#define TBOX_FLOW_STATE_MACHINE_H_20220320

#include <functional>
#include <string>

#include <tbox/base/json_fwd.h>
#include "event.h"

namespace tbox {
namespace flow {

//! HFSM，多层级有限状态机
class StateMachine {
  public:
    using StateID = int;   //! StateID =  0 的状态为终止状态，用户可以不用定义
                           //! StateID = -1 的表示无效状态

    using EventID = int;   //! EventID = 0 表示任意事件，仅在 addRoute(), addEvent() 时使用

    //! 动作执行函数
    using ActionFunc = std::function<void(Event)>;
    //! 条件判定函数，返回true表示条件成立，false表示条件不成立
    using GuardFunc  = std::function<bool(Event)>;
    //! 事件处理函数，返回<0表示不进行状态转换，返回>=0表示需要进行状态转换
    using EventFunc  = std::function<StateID(Event)>;
    //! 状态变更回调函数
    using StateChangedCallback = std::function<void(StateID/*from_state_id*/, StateID/*to_state_id*/, Event)>;

  public:
    StateMachine();
    ~StateMachine();

  public:
    /**
     * \brief   创建一个状态
     *
     * \param   state_id        状态ID号
     * \param   enter_action    进入该状态时的动作，nullptr表示无动作
     * \param   exit_action     退出该状态时的动作，nullptr表示无动作
     *
     * \return  bool    成功与否，重复创建会失败
     *
     * \note  newState() 的第一个状态，默认为起始状态
     */
    bool newState(StateID state_id,
                  const ActionFunc &enter_action,
                  const ActionFunc &exit_action,
                  const std::string &label = "");

    template <typename S>
    bool newState(S state_id,
                  const ActionFunc &enter_action,
                  const ActionFunc &exit_action,
                  const std::string &label = "") {
        return newState(static_cast<StateID>(state_id), enter_action, exit_action, label);
    }

    /**
     * \brief   添加状态转换路由
     *
     * \param   from_state_id   源状态
     * \param   event_id        事件，0表示任意事件
     * \param   to_state_id     目标状态
     * \param   guard           判定条件，nullptr表示不进行判断
     * \param   action          转换中执行的动作，nullptr表示无动作
     *
     * \return  bool    成功与否
     *                  from_state_id，to_state_id所指状态不存在时会失败
     */
    bool addRoute(StateID from_state_id,
                  EventID event_id,
                  StateID to_state_id,
                  const GuardFunc &guard,
                  const ActionFunc &action,
                  const std::string &label = "");

    template <typename S, typename E>
    bool addRoute(S from_state_id,
                  E event_id,
                  S to_state_id,
                  const GuardFunc &guard,
                  const ActionFunc &action,
                  const std::string &label = "") {
        return addRoute(static_cast<StateID>(from_state_id),
                        static_cast<EventID>(event_id),
                        static_cast<StateID>(to_state_id),
                        guard, action, label);
    }

    /**
     * \brief   添加状态的事件处理
     *
     * \param   state_id        状态
     * \param   event_id        事件，0表示任意事件
     * \param   action          事件对应的动作函数
     *
     * \return  bool    成功与否
     */
    bool addEvent(StateID state_id,
                  EventID event_id,
                  const EventFunc &action);

    template <typename S, typename E>
    bool addEvent(S state_id,
                  E event_id,
                  const EventFunc &action) {
        return addEvent(static_cast<StateID>(state_id),
                        static_cast<EventID>(event_id),
                        action);
    }

    /**
     * \brief   设置起始与终止状态
     *
     * \param   init_state_id   起始状态
     *
     * \note  如果不指定，那么第一个newState()的状态为起始状态
     */
    void setInitState(StateID init_state_id);

    template <typename S>
    void setInitState(S init_state_id) {
        return setInitState(static_cast<StateID>(init_state_id));
    }

    //! 设置子状态机
    bool setSubStateMachine(StateID state_id, StateMachine *wp_sub_sm);

    template <typename S>
    bool setSubStateMachine(S state_id, StateMachine *wp_sub_sm) {
        return setSubStateMachine(static_cast<StateID>(state_id), wp_sub_sm);
    }

    //! 设置状态变更回调
    void setStateChangedCallback(StateChangedCallback &&cb);

    //! 启动状态机
    bool start();

    //! 停止状态机
    void stop();

    //! 重启状态机
    //! 等价于先stop()再start()
    bool restart();

    /**
     * \brief   运行状态机
     *
     * \param   event_id    事件
     *
     * \return  bool    状态是否变换
     */
    bool run(Event event);

    /**
     * \brief   获取当前状态ID
     *
     * \return  >=0 当前状态ID
     * \return  -1  状态机未启动，或转换中
     */
    StateID currentState() const;
    template <typename S>
    S currentState() const { return static_cast<S>(currentState()); }

    /**
     * \brief   获取上一个状态ID
     *
     * \return  >=0 上一个状态ID
     * \return  -1  无上一个状态
     */
    StateID lastState() const;
    template <typename S>
    S lastState() const { return static_cast<S>(lastState()); }

    /**
     * \brief   获取下一个状态ID
     *
     * \return  >=0 下一个状态ID
     * \return  -1  无下一个状态
     *
     * \note    在退出状态动作与路由动作中有效
     */
    StateID nextState() const;
    template <typename S>
    S nextState() const { return static_cast<S>(nextState()); }

    //! 是否运行中
    bool isRunning() const;

    //! 是否已终止
    bool isTerminated() const;

    //! 将数据导出成JSON
    void toJson(Json &js) const;

    //! 设置状态机的名称
    void setName(const std::string &name);

  private:
    class Impl;
    Impl *impl_;
};

}
}

#endif //TBOX_FLOW_STATE_MACHINE_H_20220320
