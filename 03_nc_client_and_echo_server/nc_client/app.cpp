#include "app.h"

#include <cassert>
#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/json.hpp>

namespace nc_client {

using namespace tbox::main;
using namespace tbox::network;

void App::fillDefaultConfig(Json &cfg) const
{
    cfg["nc_client"]["server"] = "127.0.0.1:12345";
}

bool App::construct(tbox::main::Context &ctx)
{
    client_ = new TcpClient(ctx.loop());
    stdio_ = new StdioStream(ctx.loop());

    return (client_ != nullptr && stdio_ != nullptr);
}

App::~App()
{
    CHECK_DELETE_RESET_OBJ(stdio_);
    CHECK_DELETE_RESET_OBJ(client_);
}

bool App::initialize(const tbox::Json &cfg)
{
    auto js_server = cfg["nc_client"]["server"];
    if (!js_server.is_string())
        return false;

    if (!client_->initialize(SockAddr::FromString(js_server.get<std::string>())))
        return false;

    client_->bind(stdio_);
    stdio_->bind(client_);
    return true;
}

bool App::start()
{
    client_->start();
    stdio_->enable();
    return true;
}

void App::stop()
{
    client_->stop();
}

void App::cleanup()
{
    client_->cleanup();
}

}
