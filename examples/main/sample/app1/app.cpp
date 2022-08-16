#include "app.h"
#include <tbox/base/log.h>

namespace app1 {

App::App(tbox::main::Context &ctx) :
    Module("app1", ctx)
{
    LogTag();
}

App::~App()
{
    LogTag();
}

bool App::onInit(const tbox::Json &cfg)
{
    LogTag();
    return true;
}

bool App::onStart()
{
    LogTag();
    return true;
}

void App::onStop()
{
    LogTag();
}

void App::onCleanup()
{
    LogTag();
}

}
