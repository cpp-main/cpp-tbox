#include "app.h"
#include <base/log.h>

App::App(tbox::main::Context &ctx) :
    Module("app", ctx)
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
