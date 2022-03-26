#ifndef TBOX_MAIN_MODULE_H_20220326
#define TBOX_MAIN_MODULE_H_20220326

#include <vector>
#include <tbox/base/json_fwd.h>
#include "context.h"

namespace tbox {
namespace main {

//! 通用模块
class Module {
  public:
    explicit Module(Context &ctx);
    virtual ~Module();

    enum class State {
        kNone,      //!< 完成了构造，还没有initialize()
        kInited,    //!< 已初始化，还没有start()
        kRunning    //!< 已start()
    };

  public:
    bool addChild(Module *child);
    //!注意:一旦将子Module添加到父Module，子Module的生命期就由父Module管控
    //!     不可私自delete

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
