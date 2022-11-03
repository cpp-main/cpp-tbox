#ifndef TBOX_ACTION_SEQUENCE_ACTION_H_20221002
#define TBOX_ACTION_SEQUENCE_ACTION_H_20221002

#include "../action.h"

namespace tbox {
namespace action {

/**
 * bool SequenceAction(acton_vec) {
 *   for (item : action_vec)
 *     if (!item())
 *       return false;
 *   return true;
 * }
 */
class SequenceAction : public Action {
  public:
    using Action::Action;
    virtual ~SequenceAction();

    virtual std::string type() const override { return "Sequence"; }

    virtual void toJson(Json &js) const;

    int append(Action *action);

    int index() const { return index_; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual void onReset() override;

  private:
    void startOtheriseFinish();
    void onChildFinished(bool is_succ);

  private:
    size_t index_ = 0;
    std::vector<Action*> children_;
};

}
}

#endif //TBOX_ACTION_SEQUENCE_ACTION_H_20221002
