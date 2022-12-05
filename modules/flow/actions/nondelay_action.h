#ifndef TBOX_ACTION_NONDELAY_ACTION_H_20221003
#define TBOX_ACTION_NONDELAY_ACTION_H_20221003

#include "../action.h"

namespace tbox {
namespace flow {

class NondelayAction : public Action {
  public:
    using Func = std::function<bool()>;
    explicit NondelayAction(event::Loop &loop, const Func &func);

    virtual std::string type() const override { return "Nondelay"; }

  protected:
    virtual bool onStart() override;

  private:
    Func func_;
};

}
}

#endif //TBOX_ACTION_NONDELAY_ACTION_H_20221003
