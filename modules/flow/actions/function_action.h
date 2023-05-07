#ifndef TBOX_FLOW_FUNCTION_ACTION_H_20221003
#define TBOX_FLOW_FUNCTION_ACTION_H_20221003

#include "../action.h"

namespace tbox {
namespace flow {

class FunctionAction : public Action {
 public:
  using Func = std::function<bool()>;
  explicit FunctionAction(event::Loop &loop);
  void setFunc(const Func &func) { func_ = func; }

 protected:
  virtual bool onInit() override { return func_ != nullptr; }
  virtual bool onStart() override;

 private:
  Func func_;
};

}
}

#endif //TBOX_FLOW_FUNCTION_ACTION_H_20221003
