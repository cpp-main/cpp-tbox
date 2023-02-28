#include "backtrace.h"
#include <gtest/gtest.h>

#include <tbox/base/log_output.h>

namespace tbox {
namespace util {

TEST(Backtrace, _)
{
    LogOutput_Initialize();
    LogCallStack();
    LogOutput_Cleanup();
}

}
}
