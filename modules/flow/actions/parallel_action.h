#ifndef TBOX_FLOW_PARALLEL_ACTION_H_20221005
#define TBOX_FLOW_PARALLEL_ACTION_H_20221005

#include "../action.h"

#include <chrono>
#include <set>

namespace tbox {
namespace flow {

class ParallelAction : public Action {
  public:
    using Action::Action;
    virtual ~ParallelAction();

    virtual std::string type() const override { return "Parallel"; }

    int append(Action *action);

    std::set<int> succSet() const { return succ_set_; }
    std::set<int> failSet() const { return fail_set_; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    void onChildFinished(int index, bool is_succ);

  private:
    std::vector<Action*> children_;
    std::set<int> succ_set_;
    std::set<int> fail_set_;
};

}
}

#endif //TBOX_FLOW_PARALLEL_ACTION_H_20221005
