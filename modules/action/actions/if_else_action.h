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
    explicit IfElseAction(Context &ctx, const std::string &name,
        Action *cond_action, Action *if_action, Action *else_action);
    virtual ~IfElseAction();

    virtual std::string type() const override { return "IfElse"; }
    virtual void toJson(Json &js) const;

    virtual bool start() override;
    virtual bool pause() override;
    virtual bool resume() override;
    virtual bool stop() override;

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
