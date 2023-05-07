#ifndef TBOX_FLOW_COMPOSITE_ACTION_H_20221105
#define TBOX_FLOW_COMPOSITE_ACTION_H_20221105

#include "../action.h"

namespace tbox {
namespace flow {

/// 封装动作类
/**
 * 在应用中，我们经常需要定义一个动作类，它由一组动作组合而成。
 *
 * 我们可以定义一个类，继承于`CompositeAction`，然后在该类的构造函数中组装动作。
 * 最后调用`setChild()`将动作设置为CompositeAction的子动作。
 *
 * class MyAction : public CompositeAction {
 *   public:
 *     MyAction(event::Loop &loop)
 *       : CompositeAction(loop, "MyAction")
 *     {
 *       auto loop_action = LoopAction::MakeShared(loop);
 *       loop_action->append(...);
 *       loop_action->append(...);
 *       setChild(loop_action);
 *     }
 * };
 *
 * 不用重写`Action`中其它虚函数。
 */
class CompositeAction : public Action {
  public:
    using Action::Action;

    virtual void toJson(Json &js) const override;

  protected:
    void setChild(Action::SharedPtr child);

  protected:
    virtual bool onInit() override;
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action::SharedPtr child_;
};

}
}

#endif //TBOX_FLOW_COMPOSITE_ACTION_H_20221105
