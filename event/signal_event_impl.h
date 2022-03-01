#ifndef TBOX_EVENT_SIGNAL_EVENT_IMPL_H_20220301
#define TBOX_EVENT_SIGNAL_EVENT_IMPL_H_20220301

#include "signal_event.h"

namespace tbox {
namespace event {

class CommonLoop;

class SignalEventImpl : SignalEvent {
  public:
    explicit SignalEventImpl(CommonLoop *wp_loop);
    virtual ~SignalEventImpl();

  public:
    bool initialize(int signum, Mode mode) override;
    void setCallback(const CallbackFunc &cb) override { cb_ = cb; }

    bool isEnabled() const override;
    bool enable() override;
    bool disable() override;

    Loop* getLoop() const override;

  private:
    CommonLoop *wp_loop_;
    CallbackFunc cb_;
};

}
}

#endif //TBOX_EVENT_SIGNAL_EVENT_IMPL_H_20220301
