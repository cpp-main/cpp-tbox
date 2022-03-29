#include "module1.h"
#include <tbox/base/log.h>
#include "module1_1.h"
#include "module1_2.h"

Module1::Module1(tbox::main::Context &ctx)
    tbox::main::Module(ctx)
{
    add(new Module1_1(ctx), true);
    add(new Module1_2(ctx), false);
}

bool Module1::initialize(const Json &js)
{
    MODULE_INITIALIZE_BEGIN("module1");
    //!TODO
    MODULE_INITIALIZE_END();
}

bool Module1::start()
{
    MODULE_START_BEGIN();
    //!TODO
    MODULE_START_END();
}

void Module1::stop()
{
    MODULE_STOP_BEGIN();
    //!TODO
    MODULE_STOP_END();
}

void Module1::cleanup()
{
    MODULE_CLEANUP_BEGIN();
    //!TODO
    MODULE_CLEANUP_END();
}
