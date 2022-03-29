#include "app.h"
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>

namespace app2 {

App::App(tbox::main::Context &ctx) :
    Module("app2", ctx)
{
    LogTag();
}

App::~App()
{
    LogTag();
}

void App::onFillDefaultConfig(tbox::Json &cfg)
{
    cfg["ok"] = true;
}

bool App::onInitialize(const tbox::Json &cfg)
{
    if (!cfg.contains("ok"))
        return false;

    bool ok = cfg["ok"].get<bool>();
    LogTrace("ok: %s", ok ? "true" : "false");
    return ok;
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
