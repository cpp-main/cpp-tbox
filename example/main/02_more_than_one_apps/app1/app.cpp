#include "app.h"
#include <tbox/base/log.h>
#include "sub.h"

namespace app1 {

App::App(tbox::main::Context &ctx) :
    Module("app1", ctx)
{
    add(new Sub(ctx));
    LogTag();
}

App::~App()
{
    LogTag();
}

}
