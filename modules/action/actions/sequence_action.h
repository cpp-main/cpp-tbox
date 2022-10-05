#ifndef TBOX_ACTION_SLEEP_ACTION_H_20221002
#define TBOX_ACTION_SLEEP_ACTION_H_20221002

#include "../action.h"
#include <chrono>
#include <tbox/event/forward.h>

namespace tbox {
namespace action {

class SequenceAction : public Action {
  public:
    explicit SequenceAction(Context &ctx);
    virtual ~SequenceAction();

    virtual std::string type() const override { return "Sequence"; }

    bool append(Action *action);

    virtual bool start() override;
    virtual bool stop() override;

    Action* getFailAction() const;

  private:
    void startOtheriseFinish();
    void onChildFinished(bool is_done);

  private:
    size_t index_ = 0;
    std::vector<Action*> children_;
};

}
}

#endif //TBOX_ACTION_SLEEP_ACTION_H_20221002
