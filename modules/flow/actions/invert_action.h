#ifndef TBOX_ACTION_INVERT_H_20221022
#define TBOX_ACTION_INVERT_H_20221022

#include "../action.h"

namespace tbox {
namespace flow {

/**
 * bool InvertAction(child) {
 *   return !child();
 * }
 */
class InvertAction : public Action {
  public:
    explicit InvertAction(event::Loop &loop, Action *child);
    virtual ~InvertAction();

    virtual std::string type() const override { return "Invert"; }

    virtual void toJson(Json &js) const;

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action *child_;
};

}
}

#endif //TBOX_ACTION_INVERT_H_20221022
