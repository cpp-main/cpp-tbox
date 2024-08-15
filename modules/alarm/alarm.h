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
#ifndef TBOX_ALARM_ALARM_H_20221022
#define TBOX_ALARM_ALARM_H_20221022

#include <cstdint>
#include <functional>
#include <tbox/event/forward.h>

namespace tbox {
namespace alarm {

constexpr auto kSecondsOfDay = 60 * 60 * 24;
constexpr auto kSecondsOfWeek = kSecondsOfDay * 7;

/**
 * 定时器基类
 */
class Alarm
{
  public:
    explicit Alarm(event::Loop *wp_loop);
    virtual ~Alarm();

    using Callback = std::function<void()>;
    void setCallback(const Callback &cb) { cb_ = cb; }

    /**
     * \brief 设置时区
     *
     * \param offset_minutes  相对于0时区的分钟偏移。东区为正，西区为负。
     *                        如东8区为: (8 * 60) = 480。
     *
     * \note  如果不设置，默认随系统时区
     */
    void setTimezone(int offset_minutes);

    bool isEnabled() const; //! 定时器是否已使能

    bool enable();  //! 使能定时器

    bool disable(); //! 关闭定时器

    void cleanup();

    /**
     * \brief 刷新
     *
     * 为什么需要刷新呢？
     * 有时系统时钟不准，在这种情况下 enable() 的定时任务是不准的。
     * 在时钟同步完成之后应当刷新一次
     */
    void refresh();

    //! \brief  获取剩余秒数
    uint32_t remainSeconds() const;

  protected:
    /**
     * \brief 计算下一个定时触发时间点的秒数
     *
     * \param curr_local_ts   当前时区的现在的时间戳，单位：秒
     * \param next_local_ts   当前时区的下一轮的时间戳，单位：秒
     *
     * \return true   已找到
     *         false  未找到
     *
     * \note  该函数为虚函数，需要由子类对实现。具体不同类型的定时器有不同的算法
     */
    virtual bool calculateNextLocalTimeSec(uint32_t curr_local_sec, uint32_t &next_local_sec) = 0;

    //! 定时到期动作
    virtual void onTimeExpired();

    //! 激活定时器
    bool activeTimer();

    virtual bool onEnable() { return true; }
    virtual bool onDisable() { return true; }

    static bool GetCurrentUtcTime(uint32_t &utc_sec);
    static bool GetCurrentUtcTime(uint32_t &utc_sec, uint32_t &utc_usec);

  protected:
    event::Loop *wp_loop_;
    event::TimerEvent *sp_timer_ev_;

    bool using_independ_timezone_ = false;  //! 是否使用独立的时区
    int timezone_offset_seconds_ = 0;       //! 设置的时区距0时区的秒数偏移

    int cb_level_ = 0;  //!< 防回函析构计数
    Callback cb_;

    //! 状态定义
    enum class State {
      kNone,    //!< 未初始化
      kInited,  //!< 已初始化，未启动
      kRunning  //!< 已启动
    };
    State state_ = State::kNone;  //!< 当前状态

    uint32_t target_utc_sec_ = 0;
};

}
}

#endif //TBOX_ALARM_ALARM_H_20221022
