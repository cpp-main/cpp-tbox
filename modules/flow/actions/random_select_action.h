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
#ifndef TBOX_FLOW_RANDOM_SELECT_ACTION_H_20221002
#define TBOX_FLOW_RANDOM_SELECT_ACTION_H_20221002

#include "assemble_action.h"

namespace tbox {
namespace flow {

/**
 * 随机选择动作
 *
 * 模拟实现以下流程
 * bool RandomSelectAction(action_vec) {
 *   int index = rand() % action_vec.size();
 *   return action_vec[index]();
 * }
 */
class RandomSelectAction : public SerialAssembleAction {
  public:
    explicit RandomSelectAction(event::Loop &loop);
    virtual ~RandomSelectAction();

    virtual void toJson(Json &js) const override;
    virtual int addChild(Action *action) override;
    virtual bool isReady() const override;

  protected:
    virtual void onStart() override;
    virtual void onReset() override;

  private:
    std::vector<Action*> children_;
};

}
}

#endif //TBOX_FLOW_RANDOM_SELECT_ACTION_H_20221002
