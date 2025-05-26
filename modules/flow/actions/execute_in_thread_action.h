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
#ifndef TBOX_FLOW_EXECUTE_IN_THREAD_ACTION_H_20250527
#define TBOX_FLOW_EXECUTE_IN_THREAD_ACTION_H_20250527

#include "../action.h"

#include <tbox/base/lifetime_tag.hpp>
#include <tbox/eventx/thread_executor.h>

namespace tbox {
namespace flow {

//! 在子线程中执行指定的动作
class ExecuteInThreadAction : public Action {
  public:
    using Func = std::function<bool(Reason &)>;

    explicit ExecuteInThreadAction(event::Loop &loop, eventx::ThreadExecutor &thread_executor);
    explicit ExecuteInThreadAction(event::Loop &loop, eventx::ThreadExecutor &thread_executor, Func &&func);

    inline void setFunc(Func &&func) { func_ = std::move(func); }

    virtual bool isReady() const { return bool(func_); }

  protected:
    virtual void onStart() override;
    virtual void onStop() override;
    virtual void onPause() override;
    virtual void onResume() override;
    virtual void onReset() override;

    virtual void toJson(Json &js) const override;

  private:
    LifetimeTag ltt_;
    eventx::ThreadExecutor &thread_executor_;
    Func func_;

    tbox::eventx::ThreadExecutor::TaskToken task_token_;
};

}
}

#endif //TBOX_FLOW_EXECUTE_IN_THREAD_ACTION_H_20250527
