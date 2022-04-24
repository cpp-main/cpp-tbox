#ifndef TBOX_STATE_MACHINE_H_20220320
#define TBOX_STATE_MACHINE_H_20220320

#include <functional>

namespace tbox {
namespace util {

//! 状态机
class StateMachine {
  public:
    using StateID = unsigned int;   //! 注意：StateID应大于0，StateID表示不合法的状态
    using EventID = unsigned int;   //! 注意：EventID = 0 表示任意事件，仅在 addRoute() 时使用

    using ActionFunc = std::function<void()>;
    using GuardFunc  = std::function<bool()>;

    using StateChangedCallback = std::function<void(StateID/*from_state_id*/, StateID/*to_state_id*/, EventID)>;

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
     */
    bool newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action);

    template <typename S>
    bool newState(S state_id, const ActionFunc &enter_action, const ActionFunc &exit_action) {
        return newState(static_cast<StateID>(state_id), enter_action, exit_action);
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
    bool addRoute(StateID from_state_id, EventID event_id, StateID to_state_id, const GuardFunc &guard, const ActionFunc &action);

    template <typename S, typename E>
    bool addRoute(S from_state_id, E event_id, S to_state_id, const GuardFunc &guard, const ActionFunc &action) {
        return addRoute(static_cast<StateID>(from_state_id),
                        static_cast<EventID>(event_id),
                        static_cast<StateID>(to_state_id),
                        guard, action);
    }

    /**
     * \brief   设置起始与终止状态
     *
     * \param   init_state_id   起始状态，必须指定
     * \param   term_state_id   终止状态，默认为0表示无
     *
     * \return  bool    成功与否
     *                  如果 init_state_id 或 term_state_id 不存在则会返回false
     */
    bool initialize(StateID init_state_id, StateID term_state_id = 0);

    template <typename S>
    bool initialize(S init_state_id, S term_state_id) {
        return initialize(static_cast<StateID>(init_state_id), static_cast<StateID>(term_state_id));
    }

    template <typename S>
    bool initialize(S init_state_id) {
        return initialize(static_cast<StateID>(init_state_id));
    }

    //! 设置子状态机
    bool setSubStateMachine(StateID state_id, StateMachine *wp_sub_sm);

    template <typename S>
    bool setSubStateMachine(S state_id, StateMachine *wp_sub_sm) {
        return setSubStateMachine(static_cast<StateID>(state_id), wp_sub_sm);
    }

    //! 设置状态变更回调
    void setStateChangedCallback(const StateChangedCallback &cb);

    //! 启动状态机
    bool start();

    //! 停止状态机
    void stop();

    /**
     * \brief   运行状态机
     *
     * \param   event_id    事件
     *
     * \return  bool    状态是否变换
     */
    bool run(EventID event_id);

    template <typename E>
    bool run(E event_id) {
        return run(static_cast<EventID>(event_id));
    }

    /**
     * \brief   获取当前状态ID
     *
     * \return  >0 当前状态ID
     * \return  =0 状态机未启动
     */
    StateID currentState() const;

    template <typename S>
    S currentState() const {
        return static_cast<S>(currentState());
    }

    bool isTerminated() const;

  private:
    class Impl;
    Impl *impl_;
};

}
}

#endif //TBOX_STATE_MACHINE_H_20220320
