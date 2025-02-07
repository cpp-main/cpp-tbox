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
#ifndef TBOX_FLOW_LOOP_IF_ACTION_H_20221108
#define TBOX_FLOW_LOOP_IF_ACTION_H_20221108

#include "assemble_action.h"

namespace tbox {
namespace flow {

class LoopIfAction : public SerialAssembleAction {
  public:
    explicit LoopIfAction(event::Loop &loop);
    virtual ~LoopIfAction();

    virtual void toJson(Json &js) const override;

    //! role: "if", "exec"
    virtual bool setChildAs(Action *child, const std::string &role) override;

    virtual bool isReady() const override;

    //! 默认结束结果是true，如果需要可以设定
    void setFinishResult(bool succ) { finish_result_ = succ; }

  protected:
    virtual void onStart() override;
    virtual void onReset() override;

    void onIfFinished(bool is_succ, const Reason &why, const Trace &trace);
    void onExecFinished(const Reason &why, const Trace &trace);

  private:
    Action *if_action_ = nullptr;
    Action *exec_action_ = nullptr;
    bool finish_result_ = true;
};

}
}

#endif //TBOX_FLOW_LOOP_IF_ACTION_H_20221108
