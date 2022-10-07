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
    explicit ParallelAction(Context &ctx, const std::string &name);
    virtual ~ParallelAction();

    virtual std::string type() const override { return "Parallel"; }

    int append(Action *action);

    virtual bool start() override;
    virtual bool stop() override;

    std::set<int> succSet() const { return succ_set_; }
    std::set<int> failSet() const { return fail_set_; }

  private:
    void onChildFinished(int index, bool is_succ);

  private:
    std::vector<Action*> children_;
    std::set<int> succ_set_;
    std::set<int> fail_set_;
};

}
}

#endif //TBOX_ACTION_PARALLEL_ACTION_H_20221005
