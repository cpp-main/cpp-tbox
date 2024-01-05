/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <cxxabi.h>

#include <functional>
#include <exception>
#include <tbox/base/log.h>
#include <tbox/base/backtrace.h>

int main(int argc, char **argv);

namespace tbox {
namespace main {

extern void AbnormalExit();

namespace {

void Terminate()
{
    static bool terminating = false;
    if (terminating) {
        LogFatal("terminate called recursively");
        std::abort();
    }
    terminating = true;

    // Make sure there was an exception; terminate is also called for an
    // attempt to rethrow when there is no suitable exception.
    std::type_info *t = abi::__cxa_current_exception_type();
    if (t) {
        // Note that "name" is the mangled name.
        const char *name = t->name();
        const char *readable_name = name;
        int status = -1;
        char *demangled = abi::__cxa_demangle(name, 0, 0, &status);
        if (demangled != nullptr)
            readable_name = demangled;

        LogFatal("terminate called after throwing an instance of '%s'", readable_name);

        if (status == 0)
            free(demangled);

        // If the exception is derived from std::exception, we can
        // give more information.
        __try { __throw_exception_again; }
#ifdef __EXCEPTIONS
        __catch(const std::exception& e) {
            LogFatal("what(): %s", e.what());
        }
#endif
        __catch(...) { }
    } else {
        LogFatal("terminate called without an active exception");
    }

#if 1
    const std::string &stack_str = DumpBacktrace();
    LogFatal("main: <%p>\n-----call stack-----\n%s", ::main, stack_str.c_str());
#endif

    AbnormalExit();
}

}

void InstallTerminate()
{
    std::set_terminate(Terminate);
}

}
}
