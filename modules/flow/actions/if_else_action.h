#ifndef TBOX_FLOW_IF_ELSE_H_20221022
#define TBOX_FLOW_IF_ELSE_H_20221022

#include "../action.h"

namespace tbox {
namespace flow {

/**
 * bool IfElseAction(if_action, succ_action, fail_acton) {
 *   if (if_action())
 *     return succ_action();
 *   else
 *     return fail_acton();
 * }
 */
class IfElseAction : public Action {
  public:
    explicit IfElseAction(event::Loop &loop);

    virtual void toJson(Json &js) const override;

    void setIfAction(Action::SharedPtr action);
    void setSuccAction(Action::SharedPtr action);
    void setFailAction(Action::SharedPtr action);

  protected:
    virtual bool onInit() override;
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  protected:
    void onCondActionFinished(bool is_succ);

  private:
    Action::SharedPtr if_action_;
    Action::SharedPtr succ_action_;
    Action::SharedPtr fail_action_;
};

}
}

#endif //TBOX_FLOW_IF_ELSE_H_20221022
