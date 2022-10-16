#ifndef TBOX_ACTION_INVERT_H_20221022
#define TBOX_ACTION_INVERT_H_20221022

#include "../action.h"

namespace tbox {
namespace action {

/**
 * bool InvertAction(child) {
 *   return !child();
 * }
 */
class InvertAction : public Action {
  public:
    explicit InvertAction(Context &ctx, const std::string &name, Action *child);
    virtual ~InvertAction();

    virtual std::string type() const override { return "Invert"; }

    virtual void toJson(Json &js) const;

    virtual bool start() override;
    virtual bool pause() override;
    virtual bool resume() override;
    virtual bool stop() override;

  private:
    Action *child_;
};

}
}

#endif //TBOX_ACTION_INVERT_H_20221022
