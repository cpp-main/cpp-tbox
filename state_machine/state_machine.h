#ifndef TBOX_STATE_MACHINE_H_20220320
#define TBOX_STATE_MACHINE_H_20220320

#include <functional>

namespace tbox {
namespace sm {

//! 状态机
class StateMachine {
  public:
    using StateID = uint16_t;
    using EventID = uint16_t;   //! 注意，有效的EventID>0

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
     * \param   enter_action    进入该状态时的动作
     * \param   exit_action     退出该状态时的动作
     *
     * \return  bool    成功与否，重复创建会失败
     *
     * \note    默认创建的第一个状态就是初始状态
     */
    bool newState(StateID state, const ActionFunc &enter_action, const ActionFunc &exit_action);

    /**
     * \brief   添加状态转换路由
     *
     * \param   from    源状态
     * \param   event   事件
     * \param   to      目标状态
     * \param   guard   判定条件
     * \param   action  转换中执行的动作
     *
     * \return  bool    成功与否，from，to状态不存在时会失败
     */
    bool addRoute(StateID from, EventID event, StateID to, const GuardFunc &guard, const ActionFunc &action);

    /**
     * \brief   启动状态机，并指定初始状态
     */
    bool start(StateID init_state);

    /**
     * \brief   运行状态机
     *
     * \param   event   事件
     *
     * \return  bool    状态是否变换
     */
    bool run(EventID event);

    //! 获取当前状态ID
    StateID currentState() const;

  private:
    class Impl;
    Impl *impl_;
};

}
}

#endif //TBOX_STATE_MACHINE_H_20220320
