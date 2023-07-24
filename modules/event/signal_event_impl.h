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
#ifndef TBOX_EVENT_SIGNAL_EVENT_IMPL_H_20220301
#define TBOX_EVENT_SIGNAL_EVENT_IMPL_H_20220301

#include "signal_event.h"

namespace tbox {
namespace event {

class CommonLoop;

class SignalSubscribuer {
  public:
    virtual void onSignal(int signo) = 0;

  protected:
    virtual ~SignalSubscribuer() { }
};

class SignalEventImpl : public SignalEvent,
                        public SignalSubscribuer {
  public:
    explicit SignalEventImpl(CommonLoop *wp_loop, const std::string &what);
    virtual ~SignalEventImpl() override;

  public:
    virtual bool initialize(int signum, Mode mode) override;
    virtual bool initialize(const std::set<int> &sigset, Mode mode) override;
    virtual bool initialize(const std::initializer_list<int> &sigset, Mode mode) override;

    virtual void setCallback(CallbackFunc &&cb) override { cb_ = std::move(cb); }

    virtual bool isEnabled() const override { return is_enabled_; }
    virtual bool enable() override;
    virtual bool disable() override;

    virtual Loop* getLoop() const override;

  public:
    virtual void onSignal(int signo) override;

  private:
    CommonLoop *wp_loop_;
    CallbackFunc cb_;

    bool is_inited_ = false;
    bool is_enabled_ = false;

    std::set<int> sigset_;
    Mode mode_ = Mode::kPersist;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_EVENT_SIGNAL_EVENT_IMPL_H_20220301
