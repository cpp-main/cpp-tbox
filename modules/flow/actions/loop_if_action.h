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

#include "../action.h"

namespace tbox {
namespace flow {

class LoopIfAction : public Action {
  public:
    explicit LoopIfAction(event::Loop &loop,
                          Action *if_action,
                          Action *exec_action);
    virtual ~LoopIfAction();

    virtual void toJson(Json &js) const override;

    //! 默认结束结果是true，如果需要可以设定
    void setFinishResult(bool succ) { finish_result_ = succ; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

    void onIfFinished(bool is_succ);
    void onExecFinished();

  private:
    Action *if_action_;
    Action *exec_action_;
    bool finish_result_ = true;
};

}
}

#endif //TBOX_FLOW_LOOP_IF_ACTION_H_20221108
