#ifndef TBOX_ACTION_SLEEP_ACTION_H_20221002
#define TBOX_ACTION_SLEEP_ACTION_H_20221002

#include "../action.h"
#include <chrono>

namespace tbox {
namespace action {

class SleepAction : public Action {
  public:
    SleepAction(event::Loop &loop, const std::chrono::milliseconds &time_span);
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
};

}
}

#endif //TBOX_ACTION_SLEEP_ACTION_H_20221002
