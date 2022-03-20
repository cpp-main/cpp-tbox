#ifndef TBOX_STATE_MACHINE_H_20220320
#define TBOX_STATE_MACHINE_H_20220320

#include <functional>

namespace tbox {
namespace util {

//! 状态机
class StateMachine {
  public:
    using StateID = uint16_t;   //! 注意：StateID应大于0，StateID表示不合法的状态
    using EventID = uint16_t;   //! 注意：EventID = 0 表示任意事件，仅在 addRoute() 时使用

    using ActionFunc = std::function<void()>;
    using GuardFunc  = std::function<bool()>;

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
     * \note    默认创建的第一个状态就是初始状态
     */
    bool newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action);

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

    /**
     * \brief   启动状态机，并指定初始状态
     *
     * \param   init_state      初始状态
     *
     * \return  bool    成功与否
     *                  重复start()或init_state所指状态不存在时会不成功
     */
    bool start(StateID init_state);

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

    //! 获取当前状态ID
    StateID currentState() const;

  private:
    class Impl;
    Impl *impl_;
};

}
}

#endif //TBOX_STATE_MACHINE_H_20220320
