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
#ifndef TBOX_EVENT_TIMER_EVENT_IMPL_H_20200110
#define TBOX_EVENT_TIMER_EVENT_IMPL_H_20200110

#include <tbox/base/cabinet_token.h>
#include "timer_event.h"

namespace tbox {
namespace event {

class CommonLoop;

class TimerEventImpl : public TimerEvent {
  public:
    explicit TimerEventImpl(CommonLoop *wp_loop, const std::string &what);
    virtual ~TimerEventImpl() override;

  public:
    virtual bool initialize(const std::chrono::milliseconds &interval, Mode mode) override;
    virtual void setCallback(CallbackFunc &&cb) override { cb_ = std::move(cb); }

    virtual bool isEnabled() const override;
    virtual bool enable() override;
    virtual bool disable() override;

    virtual Loop* getLoop() const override;

  protected:
    void onEvent();

  private:
    CommonLoop *wp_loop_;

    bool is_inited_  = false;
    bool is_enabled_ = false;

    std::chrono::milliseconds interval_;
    Mode mode_ = Mode::kOneshot;

    CallbackFunc cb_;
    int cb_level_ = 0;

    cabinet::Token token_;
};

}
}

#endif //TBOX_EVENT_TIMER_EVENT_IMPL_H_20200110
