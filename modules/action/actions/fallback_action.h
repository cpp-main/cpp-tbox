#ifndef TBOX_ACTION_FALLBACK_ACTION_H_20221031
#define TBOX_ACTION_FALLBACK_ACTION_H_20221031

#include "../action.h"

namespace tbox {
namespace action {

/**
 * bool FallbackAction(acton_vec) {
 *   for (item : action_vec)
 *     if (item())
 *       return true;
 *   return false;
 * }
 */
class FallbackAction : public Action {
  public:
    using Action::Action;
    virtual ~FallbackAction();

    virtual std::string type() const override { return "Fallback"; }

    virtual void toJson(Json &js) const;

    int append(Action *action);

    int index() const { return index_; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;

  private:
    void startOtheriseFinish();
    void onChildFinished(bool is_succ);

  private:
    size_t index_ = 0;
    std::vector<Action*> children_;
};

}
}

#endif //TBOX_ACTION_FALLBACK_ACTION_H_20221031
