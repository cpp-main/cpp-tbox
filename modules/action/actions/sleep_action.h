#ifndef TBOX_ACTION_SLEEP_ACTION_H_20221002
#define TBOX_ACTION_SLEEP_ACTION_H_20221002

#include "../action.h"
#include <chrono>
#include <tbox/event/forward.h>

namespace tbox {
namespace action {

class SleepAction : public Action {
  public:
    SleepAction(Context &ctx, const std::string &name, const std::chrono::milliseconds &time_span);
    ~SleepAction();

    virtual std::string type() const override { return "Sleep"; }

    virtual bool start() override;
    virtual bool stop() override;

  private:
    event::TimerEvent *timer_;
};

}
}

#endif //TBOX_ACTION_SLEEP_ACTION_H_20221002
