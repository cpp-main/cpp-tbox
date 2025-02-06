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
#ifndef TBOX_FLOW_SWITCH_H_20250202
#define TBOX_FLOW_SWITCH_H_20250202

#include "assemble_action.h"

namespace tbox {
namespace flow {

/**
 * bool SwitchAction() {
 *   switch (switch_action()) {
 *     case "xxx": return xxx_action();
 *     ...
 *     default: return default_action();
 *   }
 * }
 */
class SwitchAction : public SerialAssembleAction {
  public:
    explicit SwitchAction(event::Loop &loop);
    virtual ~SwitchAction();

    virtual void toJson(Json &js) const override;

    //! role: "switch", "case:xxx", "default"
    virtual bool setChildAs(Action *child, const std::string &role) override;

    virtual bool isReady() const override;

  protected:
    virtual void onStart() override;
    virtual void onReset() override;

  protected:
    void onSwitchActionFinished(bool is_succ, const Reason &why);

  private:
    Action *switch_action_   = nullptr;
    std::map<std::string, Action*> case_actions_;
    Action *default_action_ = nullptr;
};

}
}

#endif //TBOX_FLOW_SWITCH_H_20250202
