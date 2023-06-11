#ifndef TBOX_EVENTX_TIMER_FD_H_20230607
#define TBOX_EVENTX_TIMER_FD_H_20230607

#include <functional>
#include <chrono>

#include <tbox/event/forward.h>

namespace tbox {
namespace eventx {

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
     * \param repeat    重复触发间隔时长，如果为zero()则表示不重复触发
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
