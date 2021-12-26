#include "app.h"

#include <cassert>
#include <tbox/base/log.h>

namespace echo_server {

using namespace tbox::main;
using namespace tbox::network;

App::App(Context &ctx) :
    server_(new TcpServer(ctx.loop()))
{
    assert(server_ != nullptr);
}

App::~App()
{
    delete server_;
}

bool App::initialize()
{
    if (!server_->initialize(SockAddr::FromString("127.0.0.1:12345")))
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
