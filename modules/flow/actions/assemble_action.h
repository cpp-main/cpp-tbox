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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#ifndef TBOX_FLOW_ASSEMBLE_ACTION_H_20240305
#define TBOX_FLOW_ASSEMBLE_ACTION_H_20240305

#include "../action.h"

namespace tbox {
namespace flow {

class AssembleAction : public Action {
  public:
    using Action::Action;

  public:
    //!< 设置子动作
    virtual int  addChild(Action *child);
    virtual int  addChildAs(Action *child, const std::string &role);
    virtual bool setChild(Action *child);
    virtual bool setChildAs(Action *child, const std::string &role);

    //! 设置最终（在被停止或自然结束时）的回调函数
    using FinalCallback = std::function<void()>;
    void setFinalCallback(FinalCallback &&cb) { final_cb_ = std::move(cb); }

  protected:
    virtual void onFinal() override;

  private:
    FinalCallback final_cb_;
};

//! 串行执行的组装动作
class SerialAssembleAction : public AssembleAction {
  public:
    using AssembleAction::AssembleAction;

  protected:
    virtual void onPause() override;
    virtual void onResume() override;
    virtual void onStop() override;
    virtual void onReset() override;

  protected:
    using ChildFinishFunc = std::function<void()>;

    bool startThisAction(Action *action);
    void stopCurrAction();

    //! 子动作结束事件处理
    bool handleChildFinishEvent(ChildFinishFunc &&child_finish_func);
    //! 最后一个动作结束的缺省处理
    void onLastChildFinished(bool is_succ, const Reason &reason, const Trace &trace);

  private:
    Action *curr_action_ = nullptr;     //! 当前正在执行的动作
    ChildFinishFunc child_finish_func_; //! 上一个动用缓存的finish事件
};

}
}

#endif //TBOX_FLOW_ASSEMBLE_ACTION_H_20240305
