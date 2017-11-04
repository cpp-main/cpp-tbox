#ifndef TBOX_BASE_SCOPE_EXIT_H_20171104
#define TBOX_BASE_SCOPE_EXIT_H_20171104

#include <functional>
#include "defines.h"

namespace tbox {

//! 退出区域的时候执行的动作
class ScopeExitAction {
    using ScopeExitFunc = std::function<void()>;
    ScopeExitFunc func_;

  public:
    ScopeExitAction(const ScopeExitFunc &func) : func_(func) { }

    NONCOPYABLE(ScopeExitAction);
    IMMOVABLE(ScopeExitAction);

    ~ScopeExitAction() { if (func_) func_(); }

    void cancel() { func_ = nullptr; }
};

}

#define _ScopeExitActionName_1(line) tbox::ScopeExitAction _scope_exit_action_##line
#define _ScopeExitActionName_0(line) _ScopeExitActionName_1(line)

#define SetScopeExitAction(...) _ScopeExitActionName_0(__LINE__) (__VA_ARGS__)

#endif //TBOX_BASE_SCOPE_EXIT_H_20171104
