#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "backtrace.h"

namespace tbox {
namespace util {

std::string DumpCallStack(const unsigned int max_frames)
{
    Dl_info info;

    void *callstack[max_frames];
    std::ostringstream oss;

    unsigned int number_frames = ::backtrace(callstack, max_frames);
    char **symbols = ::backtrace_symbols(callstack, number_frames);

    for (unsigned int i = 0; i < number_frames; ++i) {
        /// try to translate an address to symbolic information
        std::string func_name;

        if (dladdr(callstack[i], &info)) {
            int status = -1;
            char *demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            if (demangled != nullptr) {
                func_name = demangled;
                free(demangled);
            } else if (info.dli_sname != nullptr) {
                func_name = info.dli_sname;
            } else {
                func_name = "null";
            }
        } else {
            func_name = (symbols != nullptr ? symbols[i] : "null");
        }

        oss << '[' << std::setw(2) << i << "]: " << func_name << " <" << callstack[i] << '>' << std::endl;
    }

    if (symbols != nullptr)
        free(symbols);

    if (number_frames >= max_frames)
        oss << "[truncated]" << std::endl;

    return oss.str();
}

}
}
