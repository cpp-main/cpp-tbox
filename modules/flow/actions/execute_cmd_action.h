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
#ifndef TBOX_FLOW_EXECUTE_CMD_ACTION_H_20250526
#define TBOX_FLOW_EXECUTE_CMD_ACTION_H_20250526

#include "../action.h"

#include <tbox/base/lifetime_tag.hpp>
#include <tbox/eventx/thread_executor.h>

namespace tbox {
namespace flow {

//! 执行System命令的动作
class ExecuteCmdAction : public Action {
  public:
    explicit ExecuteCmdAction(event::Loop &loop, eventx::ThreadExecutor &thread_executor);
    explicit ExecuteCmdAction(event::Loop &loop, eventx::ThreadExecutor &thread_executor, const std::string &cmd);

    inline void setCmd(const std::string &cmd) { cmd_ = cmd; }

    inline int getReturnCode() const { return reture_code_; }
    inline std::string getStdOutput() const { return std_output_; }

    virtual bool isReady() const { return !cmd_.empty(); }

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
    std::string cmd_;

    tbox::eventx::ThreadExecutor::TaskToken task_token_;
    int reture_code_ = 0;
    std::string std_output_;
};

}
}

#endif //TBOX_FLOW_EXECUTE_CMD_ACTION_H_20250526
