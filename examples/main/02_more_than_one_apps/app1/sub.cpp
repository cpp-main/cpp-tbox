#include "sub.h"
#include <tbox/base/log.h>

namespace app1 {

Sub::Sub(tbox::main::Context &ctx) :
    Module("sub", ctx)
{
    LogTag();
}

Sub::~Sub()
{
    LogTag();
}

bool Sub::onInit(const tbox::Json &cfg)
{
    LogTag();
    return true;
}

bool Sub::onStart()
{
    LogTag();
    return true;
}

void Sub::onStop()
{
    LogTag();
}

void Sub::onCleanup()
{
    LogTag();
}

}
