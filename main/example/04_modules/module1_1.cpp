#include "module1_1.h"
#include <tbox/base/log.h>

Module1_1::Module1(tbox::main::Context &ctx)
    tbox::main::Module(ctx)
{ }

bool Module1_1::initialize(const Json &js)
{
    MODULE_INITIALIZE_BEGIN("module1_1_1");
    LogTag();
    MODULE_INITIALIZE_END();
}

bool Module1_1::start()
{
    MODULE_START_BEGIN();
    LogTag();
    MODULE_START_END();
}

void Module1_1::stop()
{
    MODULE_STOP_BEGIN();
    LogTag();
    MODULE_STOP_END();
}

void Module1_1::cleanup()
{
    MODULE_CLEANUP_BEGIN();
    LogTag();
    MODULE_CLEANUP_END();
}
