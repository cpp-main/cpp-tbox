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
    bool addChild(Module *child, bool required = true);
    //!注意:一旦将子Module添加到父Module，子Module的生命期就由父Module管控
    //!     不可私自delete

    virtual bool initialize(const Json &js);
    virtual bool start();
    virtual void stop();
    virtual void cleanup();

    inline State state() const { return state_; }
    inline Context& ctx() const { return ctx_; }

  protected:
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

#define MODULE_INITIALIZE_BEGIN(cfg_field_name) \
    if (state_ != State::kNone) \
        return false; \
    if (!js.contains(cfg_field_name)) \
        return false; \
    const Json &js_this = js[cfg_field_name];

#define MODULE_INITIALIZE_END() \
    if (!Module::initialize(js_this)) \
        return false; \
    state_ = State::kInited; \
    return true;

#define MODULE_START_BEGIN() \
    if (state_ != State::kInited) \
        return false;

#define MODULE_START_END() \
    if (!Module::start()) \
        return false; \
    state_ = State::kRunning; \
    return true;

#define MODULE_STOP_BEGIN() \
    if (state_ != State::kRunning) \
        return; \
    Module::stop();

#define MODULE_STOP_END() \
    state_ = State::kInited;

#define MODULE_CLEANUP_BEGIN() \
    if (state_ != State::kInited) \
        return; \
    Module::cleanup();

#define MODULE_CLEANUP_END() \
    state_ = State::kNone;

#endif //TBOX_MAIN_MODULE_H_20220326
