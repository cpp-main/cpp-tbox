#ifndef TBOX_ACTION_PARALLEL_ACTION_H_20221005
#define TBOX_ACTION_PARALLEL_ACTION_H_20221005

#include "../action.h"
#include <chrono>
#include <set>
#include <tbox/event/forward.h>

namespace tbox {
namespace action {

class ParallelAction : public Action {
  public:
    explicit ParallelAction(Context &ctx);
    virtual ~ParallelAction();

    virtual std::string type() const override { return "Parallel"; }

    int append(Action *action);

    virtual bool start() override;
    virtual bool stop() override;

  private:
    void onChildFinished(int index, bool is_done);

  private:
    std::vector<Action*> children_;
    std::set<int> done_set_;
    std::set<int> fail_set_;
};

}
}

#endif //TBOX_ACTION_PARALLEL_ACTION_H_20221005
