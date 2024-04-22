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
#ifndef TBOX_FLOW_FUNCTION_ACTION_H_20221003
#define TBOX_FLOW_FUNCTION_ACTION_H_20221003

#include "../action.h"

namespace tbox {
namespace flow {

class FunctionAction : public Action {
  public:
    using Func = std::function<bool()>;
    using FuncWithReason = std::function<bool(Reason &)>;

    explicit FunctionAction(event::Loop &loop);
    explicit FunctionAction(event::Loop &loop, Func &&func);
    explicit FunctionAction(event::Loop &loop, FuncWithReason &&func);

    virtual bool isReady() const { return bool(func_) || bool(func_with_reason_); }

    inline void setFunc(Func &&func) { func_ = std::move(func); }
    inline void setFunc(FuncWithReason &&func) { func_with_reason_ = std::move(func); }

  protected:
    virtual void onStart() override;

  private:
    Func func_;
    FuncWithReason func_with_reason_;
};

}
}

#endif //TBOX_FLOW_FUNCTION_ACTION_H_20221003
