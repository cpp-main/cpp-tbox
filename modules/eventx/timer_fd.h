#ifndef TBOX_EVENTX_TIMER_FD_H_20230607
#define TBOX_EVENTX_TIMER_FD_H_20230607

#include <functional>
#include <chrono>

#include <tbox/event/forward.h>

namespace tbox {
namespace eventx {

class TimerFd {
  public:
    TimerFd(tbox::event::Loop *loop, const std::string &what);
    ~TimerFd();

  public:
    /// 初始化
    /**
     * \param first     首次延迟时长
     * \param repeat    重重间隔时长，如果不指定则表示单次定时
     */
    bool initialize(const std::chrono::nanoseconds first,
                    const std::chrono::nanoseconds repeat = std::chrono::nanoseconds::zero());
    void cleanup();

    using Callback = std::function<void ()>;
    void setCallback(Callback &&cb);

    bool isEnabled() const;
    bool enable();
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
