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
#ifndef TBOX_FLOW_DUMMY_ACTION_H_20241107
#define TBOX_FLOW_DUMMY_ACTION_H_20241107

#include "../action.h"

namespace tbox {
namespace flow {

//! 木偶动作
//! 其自身不会主动发起结束动作，也不会触发阻塞。由外部通过调用emitFinish()与emitBlock()来实现
class DummyAction : public Action {
  public:
    explicit DummyAction(event::Loop &loop) : Action(loop, "Dummy") {}

    virtual bool isReady() const { return true; }

    using Callback = std::function<void()>;

    inline void setStartCallback(Callback &&cb) { start_cb_ = std::move(cb); }
    inline void setStopCallback(Callback &&cb) { stop_cb_ = std::move(cb); }
    inline void setPauseCallback(Callback &&cb) { pause_cb_ = std::move(cb); }
    inline void setResumeCallback(Callback &&cb) { resume_cb_ = std::move(cb); }
    inline void setResetCallback(Callback &&cb) { reset_cb_ = std::move(cb); }

    inline void emitFinish(bool is_succ, const Reason &reason = Reason()) { finish(is_succ, reason); }
    inline void emitBlock(const Reason &reason) { block(reason); }

  protected:
    virtual void onStart() override;
    virtual void onStop() override;
    virtual void onPause() override;
    virtual void onResume() override;
    virtual void onReset() override;

  private:
    Callback start_cb_;
    Callback stop_cb_;
    Callback pause_cb_;
    Callback resume_cb_;
    Callback reset_cb_;
};

}
}

#endif //TBOX_FLOW_DUMMY_ACTION_H_20241107
