#ifndef TBOX_ACTION_IF_ELSE_H_20221022
#define TBOX_ACTION_IF_ELSE_H_20221022

#include "../action.h"

namespace tbox {
namespace action {

/**
 * bool IfElseAction(cond_action, if_action, else_acton) {
 *   if (cond_action())
 *     return if_action();
 *   else
 *     return else_acton();
 * }
 */
class IfElseAction : public Action {
  public:
    explicit IfElseAction(event::Loop &loop, Action *cond_action,
                          Action *if_action, Action *else_action);
    virtual ~IfElseAction();

    virtual std::string type() const override { return "IfElse"; }
    virtual void toJson(Json &js) const;

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  protected:
    void onCondActionFinished(bool is_succ);

  private:
    Action *cond_action_;
    Action *if_action_;
    Action *else_action_;

    bool is_cond_done_ = false;
    bool is_cond_succ_ = false;
};

}
}

#endif //TBOX_ACTION_IF_ELSE_H_20221022
