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
#ifndef TBOX_EVENT_EPOLL_FD_EVENT_H_20220110
#define TBOX_EVENT_EPOLL_FD_EVENT_H_20220110

#include "../../fd_event.h"
#include "types.h"

namespace tbox {
namespace event {

class  EpollLoop;

class EpollFdEvent : public FdEvent {
  public:
    explicit EpollFdEvent(EpollLoop *wp_loop, const std::string &what);
    virtual ~EpollFdEvent() override;

  public:
    virtual bool initialize(int fd, short events, Mode mode) override;
    virtual void setCallback(CallbackFunc &&cb) override { cb_ = std::move(cb); }

    virtual bool isEnabled() const override{ return is_enabled_; }
    virtual bool enable() override;
    virtual bool disable() override;

    virtual Loop* getLoop() const override;

  public:
    static void OnEventCallback(uint32_t events, void *obj);

  protected:
    void reloadEpoll();
    void onEvent(short events);

  private:
    EpollLoop *wp_loop_;
    bool is_stop_after_trigger_ = false;

    int fd_ = -1;
    uint32_t events_ = 0;
    bool is_enabled_ = false;

    CallbackFunc cb_;
    EpollFdSharedData *d_ = nullptr;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_EVENT_EPOLL_FD_EVENT_H_20220110
