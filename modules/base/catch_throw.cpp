#include "catch_throw.h"

#include <cxxabi.h>
#include <signal.h>

#include "log.h"
#include "backtrace.h"

namespace tbox {

namespace {

void CatchType()
{
    std::type_info *t = abi::__cxa_current_exception_type();
    if (t != nullptr) {
        // Note that "name" is the mangled name.
        const char *name = t->name();
        const char *readable_name = name;
        int status = -1;

        char *demangled = abi::__cxa_demangle(name, 0, 0, &status);
        if (demangled != nullptr)
            readable_name = demangled;

        LogWarn(" Catch: '%s'", readable_name);

        if (status == 0)
            free(demangled);
    }
}

}

bool CatchThrow(const std::function<void()> &func,
                bool print_backtrace, bool abort_process)
{
    try {
        if (func)
            func();
        return false;

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

    if (print_backtrace) {
        const std::string &stack_str = DumpBacktrace();
        LogWarn("----call stack-----\n%s", stack_str.c_str());
    }

    if (abort_process) {
        LogFatal("Process abort!");
        signal(SIGABRT, SIG_DFL);
        std::abort();
    }

    return true;
}

bool CatchThrowQuietly(const std::function<void()> &func)
{
    try {
        if (func)
            func();
        return false;

    } catch (...) {
        return true;
    }
}

}
