#ifndef TBOX_STATE_MACHINE_H_20220320
#define TBOX_STATE_MACHINE_H_20220320

#include <tbox/base/cabinet_token.h>
#include <functional>

namespace tbox {
namespace sm {

//! 状态机
class StateMachine {
  public:
    using StateToken = cabinet::Token;

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
     * \return  StateToken      状态Token，用于引用
     */
    StateToken newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action);

     //! 设置初始状态
    void setInitState(StateToken init_state);

    //! 设置终止状态
    void setTermState(StateToken init_state);

    /**
     * \brief   添加状态转换路由
     *
     * \param   from    源状态
     * \param   event   事件
     * \param   to      目标状态
     * \param   guard   判定条件
     * \param   action  转换中执行的动作
     *
     * \return  bool    成功与否
     */
    bool addRoute(StateToken from, EventID event, StateToken to, const GuardFunc &guard, const ActionFunc &action);

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
