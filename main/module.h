#ifndef TBOX_MAIN_MODULE_H_20220326
#define TBOX_MAIN_MODULE_H_20220326

#include <vector>
#include <tbox/base/json_fwd.h>
#include "context.h"

namespace tbox {
namespace main {

//! 模块
class Module {
  public:
    explicit Module(const std::string &name, Context &ctx);
    virtual ~Module();

    enum class State {
        kNone,      //!< 完成了构造，还没有initialize()
        kInited,    //!< 已初始化，还没有start()
        kRunning    //!< 已start()
    };

  public:
    bool addChild(Module *child, bool required = true);
    //!注意:一旦将子Module添加到父Module，子Module的生命期就由父Module管控
    //!     不可私自delete

    bool initialize(const Json &js_parent);
    bool start();
    void stop();
    void cleanup();

    inline State state() const { return state_; }
    inline Context& ctx() const { return ctx_; }

  protected:
    virtual bool onInitialize(const Json &js_this) { return true; }
    virtual bool onStart() { return true; }
    virtual void onStop() { }
    virtual void onCleanup() { }

  private:
    std::string name_;
    Context &ctx_;

    struct ModuleItem {
        Module *module_ptr;
        bool    required;   //! 是否为必须
    };
    std::vector<ModuleItem> children_;
    State state_ = State::kNone;
};

}
}

#endif //TBOX_MAIN_MODULE_H_20220326
