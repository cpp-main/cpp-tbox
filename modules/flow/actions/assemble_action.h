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

}
}

#endif //TBOX_FLOW_ASSEMBLE_ACTION_H_20240305
