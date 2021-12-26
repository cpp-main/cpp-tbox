#include "app.h"
#include <tbox/base/log.h>

namespace app2 {

App::App(tbox::main::Context &ctx)
{
    LogTag();
}

App::~App()
{
    LogTag();
}

bool App::initialize()
{
    LogTag();
    return true;
}

bool App::start()
{
    LogTag();
    return true;
}

void App::stop()
{
    LogTag();
}

void App::cleanup()
{
    LogTag();
}

}
