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
#ifndef TBOX_EVENTX_TIMER_FD_H_20230607
#define TBOX_EVENTX_TIMER_FD_H_20230607

#include <string>
#include <chrono>
#include <functional>

#include <tbox/event/forward.h>

namespace tbox {
namespace eventx {

/**
 * 基于 timerfd 的定时器类
 */
class TimerFd {
  public:
    /// 构造
    /**
     * \param loop      事件驱动Loop
     * \param what      标记，用于问题追踪
     */
    TimerFd(tbox::event::Loop *loop, const std::string &what = "TimerFd");
    ~TimerFd();

  public:
    /// 初始化
    /**
     * \param first     首次触发间隔时长
     * \param repeat    重复触发间隔时长
     *
     * \note
     *  1.如果为周期性定时器，将first与repeat均设置为触发周期即可
     *  2.如果为单次定时器，指定first为延时时长，repeat填zero即可
     *  3.first不能设置为zero()，否则定时器不工作
     */
    bool initialize(const std::chrono::nanoseconds first,
                    const std::chrono::nanoseconds repeat = std::chrono::nanoseconds::zero());

    /// 清理
    void cleanup();

    /// 设置回调
    using Callback = std::function<void ()>;
    void setCallback(Callback &&cb);

    /// 检查是否已开启
    bool isEnabled() const;

    /// 开启
    bool enable();

    /// 关闭
    bool disable();

    /// 获取距下一次触发剩余的时长
    std::chrono::nanoseconds remainTime() const;

  private:
    void onEvent(short events);

  private:
    struct Data;
    Data *d_;
};

}
}

#endif //TBOX_EVENTX_TIMER_FD_H_20230607
