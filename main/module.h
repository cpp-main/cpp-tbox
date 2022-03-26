#ifndef TBOX_MAIN_MODULE_H_20220326
#define TBOX_MAIN_MODULE_H_20220326

#include <vector>
#include <tbox/base/json_fwd.h>
#include "context.h"

namespace tbox {
namespace main {

class Module {
  public:
    explicit Module(Context &ctx);
    virtual ~Module();

    enum class State {
        kNone,
        kInited,
        kRunning
    };

  public:
    bool addChild(Module *child);

    virtual bool initialize(const Json &js);
    virtual bool start();
    virtual void stop();
    virtual void cleanup();

    inline State state() const { return state_; }
    inline Context& ctx() const { return ctx_; }

  private:
    Context &ctx_;
    std::vector<Module*> children_;
    State state_ = State::kNone;
};

}
}

#endif //TBOX_MAIN_MODULE_H_20220326
