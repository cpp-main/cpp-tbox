#ifndef TBOX_EVENT_TIMER_EVENT_IMPL_H_20200110
#define TBOX_EVENT_TIMER_EVENT_IMPL_H_20200110

#include <base/cabinet_token.h>
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
