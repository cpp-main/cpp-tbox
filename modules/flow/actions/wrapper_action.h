#ifndef TBOX_FLOW_WRAPPER_ACTION_H_20221105
#define TBOX_FLOW_WRAPPER_ACTION_H_20221105

#include "../action.h"

namespace tbox {
namespace flow {

/// 封装动作类
/**
 * 在应用中，我们经常需要定义一个动作类，它由一组动作组合而成。
 *
 * 我们可以定义一个类，继承于`WrapperAction`，然后在该类的构造函数中组装动作。
 * 最后调用`setChild()`将动作设置为WrapperAction的子动作。
 *
 * class MyAction : public WrapperAction {
 *   public:
 *     MyAction(event::Loop &loop)
 *       : WrapperAction(loop, "MyAction")
 *     {
 *       auto loop_action = new LoopAction(loop);
 *       loop_action->append(...);
 *       loop_action->append(...);
 *       setChild(loop_action);
 *     }
 * };
 *
 * 不用重写`Action`中其它虚函数。
 */
class WrapperAction : public Action {
  public:
    using Action::Action;
    virtual ~WrapperAction();

  protected:
    void setChild(Action *child);

  protected:
    virtual void toJson(Json &js) const override;

    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action *child_ = nullptr;
};

}
}

#endif //TBOX_FLOW_WRAPPER_ACTION_H_20221105
