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
#ifndef TBOX_FLOW_SLEEP_ACTION_H_20221002
#define TBOX_FLOW_SLEEP_ACTION_H_20221002

#include "../action.h"
#include <chrono>

namespace tbox {
namespace flow {

class SleepAction : public Action {
  public:
    using Generator = std::function<std::chrono::milliseconds ()>;

    SleepAction(event::Loop &loop, const std::chrono::milliseconds &time_span);
    SleepAction(event::Loop &loop, const Generator &gen);

    virtual ~SleepAction();

    virtual bool isReady() const { return true; }

  protected:
    virtual void onStart() override;
    virtual void onStop() override;
    virtual void onPause() override;
    virtual void onResume() override;
    virtual void onReset() override;

  private:
    event::TimerEvent *timer_;
    std::chrono::milliseconds time_span_;
    Generator gen_;

    std::chrono::steady_clock::time_point finish_time_;
    std::chrono::milliseconds remain_time_span_;
};

}
}

#endif //TBOX_FLOW_SLEEP_ACTION_H_20221002
