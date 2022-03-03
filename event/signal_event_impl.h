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
    explicit SignalEventImpl(CommonLoop *wp_loop);
    virtual ~SignalEventImpl();

  public:
    bool initialize(int signum, Mode mode) override;
    bool initialize(const std::set<int> &sigset, Mode mode) override;

    void setCallback(const CallbackFunc &cb) override { cb_ = cb; }

    bool isEnabled() const override { return is_enabled_; }
    bool enable() override;
    bool disable() override;

    Loop* getLoop() const override;

  public:
    void onSignal(int signo) override;

  private:
    CommonLoop *wp_loop_;
    CallbackFunc cb_;

    bool is_inited_ = false;
    bool is_enabled_ = false;

    std::set<int> sigset_;
    Mode mode_ = Mode::kPersist;
};

}
}

#endif //TBOX_EVENT_SIGNAL_EVENT_IMPL_H_20220301
