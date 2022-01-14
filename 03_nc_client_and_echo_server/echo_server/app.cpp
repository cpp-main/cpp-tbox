#include "app.h"

#include <cassert>
#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/json.hpp>

namespace echo_server {

using namespace tbox::main;
using namespace tbox::network;

void App::fillDefaultConfig(Json &cfg) const
{
    cfg["echo_server"]["bind"] = "127.0.0.1:12345";
}

bool App::construct(Context &ctx)
{
    server_ = new TcpServer(ctx.loop());
    return server_ != nullptr;
}

App::~App()
{
    CHECK_DELETE_RESET_OBJ(server_);
}

bool App::initialize(const tbox::Json &cfg)
{
    auto js_bind = cfg["echo_server"]["bind"];
    if (!js_bind.is_string())
        return false;

    if (!server_->initialize(SockAddr::FromString(js_bind.get<std::string>())))
        return false;

    server_->setReceiveCallback(
        [this] (const TcpServer::Client &client, Buffer &buff) {
            server_->send(client, buff.readableBegin(), buff.readableSize());
            buff.hasReadAll();
        }, 0
    );
    return true;
}

bool App::start()
{
    return server_->start();
}

void App::stop()
{
    server_->stop();
}

void App::cleanup()
{
    server_->cleanup();
}

}
