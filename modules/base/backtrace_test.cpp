#include "backtrace.h"
#include <gtest/gtest.h>

#include "log_output.h"

namespace tbox {

TEST(Backtrace, _)
{
    LogOutput_Enable();
    LogBacktrace();
    LogOutput_Disable();
}

}
