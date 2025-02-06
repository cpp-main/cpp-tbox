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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_FLOW_IF_THEN_ACTION_H_20250204
#define TBOX_FLOW_IF_THEN_ACTION_H_20250204

#include "assemble_action.h"
#include <vector>

namespace tbox {
namespace flow {

/**
 * bool IfThenAction() {
 *   if (if_action_1())
 *     return then_action_1();
 *   else if (if_action_2())
 *     return then_acton_2();
 *   ...
 *   else if (if_action_N())
 *     return then_acton_N();
 *   else
 *     return false;
 * }
 *
 * 实现逻辑：
 * if (if_action()) {
 *   return then_action();
 * }
 * return false;
 * --------------------------------------------------
 * auto if_then_action = new IfThenAction(*loop);
 * auto if_action = new FunctionAction(*loop, [] { return xxxx; });
 * auto then_action = new FunctionAction(*loop, [] { return yyyy; });
 * if_then_action->addChildAs(if_action, "if");
 * if_then_action->addChildAs(then_action, "then");
 *
 * 实现逻辑：
 * if (if_action()) {
 *   return true;
 * } else {
 *   return else_action();
 * }
 * 等价于：
 * if (if_action()) {
 *   return succ_action();
 * } else if (succ_action()) {
 *   return else_action();
 * }
 * --------------------------------------------------
 * auto if_then_action = new IfThenAction(*loop);
 * auto if_action = new FunctionAction(*loop, [] { return xxxx; });
 * auto else_action = new FunctionAction(*loop, [] { return yyyy; });
 * if_then_action->addChildAs(if_action, "if");
 * if_then_action->addChildAs(new SuccAction(*loop), "then");
 * if_then_action->addChildAs(new SuccAction(*loop), "if");
 * if_then_action->addChildAs(else_action, "then");
 */
class IfThenAction : public SerialAssembleAction {
  public:
    explicit IfThenAction(event::Loop &loop);
    virtual ~IfThenAction();

    virtual void toJson(Json &js) const override;

    int addChildAs(Action *child, const std::string &role);

    virtual bool isReady() const override;

  protected:
    virtual void onStart() override;
    virtual void onReset() override;

  protected:
    void doStart();
    void onIfActionFinished(bool is_succ);

  private:
    using IfThenActionPair = std::pair<Action*, Action*>;

    IfThenActionPair tmp_{ nullptr, nullptr };
    std::vector<IfThenActionPair> if_then_actions_;

    size_t index_ = 0;
};

}
}

#endif //TBOX_FLOW_IF_THEN_ACTION_H_20250204
