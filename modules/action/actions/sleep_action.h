#ifndef TBOX_ACTION_SLEEP_ACTION_H_20221002
#define TBOX_ACTION_SLEEP_ACTION_H_20221002

#include "../action.h"
#include <chrono>

namespace tbox {
namespace action {

class SleepAction : public Action {
  public:
    using Generator = std::function<std::chrono::milliseconds ()>;

    SleepAction(event::Loop &loop, const std::chrono::milliseconds &time_span);
    SleepAction(event::Loop &loop, const Generator &gen);

    ~SleepAction();

    virtual std::string type() const override { return "Sleep"; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    event::TimerEvent *timer_;
    std::chrono::milliseconds time_span_;
    Generator gen_;

    std::chrono::steady_clock::time_point finish_time_;
    std::chrono::milliseconds remain_time_span_;
};

}
}

#endif //TBOX_ACTION_SLEEP_ACTION_H_20221002
