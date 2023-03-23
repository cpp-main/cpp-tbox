#include "safe_execute.h"

#include <cxxabi.h>
#include <signal.h>

#include <tbox/base/log.h>
#include <tbox/util/backtrace.h>

namespace tbox {
namespace util {

void CatchType() {
    std::type_info *t = abi::__cxa_current_exception_type();
    if (t != nullptr) {
        // Note that "name" is the mangled name.
        const char *name = t->name();
        const char *readable_name = name;
        int status = -1;

        char *demangled = abi::__cxa_demangle(name, 0, 0, &status);
        if (demangled != nullptr)
            readable_name = demangled;

        LogWarn("  Catch: '%s'", readable_name);

        if (status == 0)
            free(demangled);
    }
}

bool SafeExecute(const std::function<void()> &func, int flags)
{
    try {
        if (func)
            func();
        return true;

    } catch (const std::exception &e) {
        CatchType();
        LogWarn("what(): %s", e.what());
    } catch (const char *e) {
        CatchType();
        LogWarn("value: %s", e);
    } catch (int e) {
        CatchType();
        LogWarn("value: %d", e);
    } catch (double e) {
        CatchType();
        LogWarn("value: %f", e);
    } catch (const std::string &e) {
        CatchType();
        LogWarn("value: %s", e.c_str());
    } catch (...) {
        CatchType();
        LogWarn("can't print value");
    }

    if (flags & SAFE_EXECUTE_PRINT_STACK) {
        const std::string &stack_str = util::DumpCallStack(64);
        LogWarn("----call stack-----\n%s", stack_str.c_str());
    }

    if (flags & SAFE_EXECUTE_ABORT) {
        LogFatal("Process abort!");
        signal(SIGABRT, SIG_DFL);
        std::abort();
    }

    return false;
}

}
}
