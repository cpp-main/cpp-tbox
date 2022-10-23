#ifndef TBOX_TIMER_TIMER_H_20221022
#define TBOX_TIMER_TIMER_H_20221022

#include <string>

#include <functional>
#include <tbox/event/forward.h>

namespace tbox {
namespace timer {

/**
 * 定时器基类
 */
class Timer
{
  public:
    explicit Timer(event::Loop *wp_loop);
    virtual ~Timer();

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

  protected:
    /**
     * \brief 计算下一个定时触发时间距当前的秒数
     *
     * \param curr_local_ts   当前时区的时间戳，单位：秒
     *
     * \return  >=0， 下一个定时触发时间距当前的秒数
     *          -1，  没有找到下一个定时的触发时间点
     *
     * \note  该函数为虚函数，需要由子类对实现。具体不同类型的定时器有不同的算法
     */
    virtual int calculateWaitSeconds(uint32_t curr_local_ts) = 0;
    virtual void onTimeExpired();

    //! 激活定时器
    bool activeTimer();

    virtual bool onEnable() { return true; }
    virtual bool onDisable() { return true; }
    virtual void onCleanup() {}

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
};

}
}

#endif //TBOX_TIMER_TIMER_H_20221022
