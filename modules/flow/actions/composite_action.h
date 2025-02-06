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
#ifndef TBOX_FLOW_COMPOSITE_ACTION_H_20221105
#define TBOX_FLOW_COMPOSITE_ACTION_H_20221105

#include "assemble_action.h"

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
 *       auto loop_action = new LoopAction(loop);
 *       loop_action->append(...);
 *       loop_action->append(...);
 *       setChild(loop_action);
 *     }
 * };
 *
 * 不用重写`Action`中其它虚函数。
 */
class CompositeAction : public SerialAssembleAction {
  public:
    using SerialAssembleAction::SerialAssembleAction;
    virtual ~CompositeAction();

  public:
    virtual void toJson(Json &js) const override;

    virtual bool setChild(Action *child) override;
    virtual bool isReady() const override;

  protected:
    virtual void onStart() override;
    virtual void onReset() override;
    virtual void onFinished(bool is_succ, const Reason &why, const Trace &trace) override;

  private:
    Action *child_ = nullptr;
};

}
}

#endif //TBOX_FLOW_COMPOSITE_ACTION_H_20221105
