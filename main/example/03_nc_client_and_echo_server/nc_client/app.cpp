#include "app.h"

#include <cassert>
#include <tbox/base/log.h>

namespace nc_client {

using namespace tbox::main;
using namespace tbox::network;

App::App(tbox::main::Context &ctx) :
    client_(new TcpClient(ctx.loop())),
    stdio_(new StdioStream(ctx.loop()))
{
    assert(client_ != nullptr);
}

App::~App()
{
    delete stdio_;
    delete client_;
}

bool App::initialize(const tbox::Json &cfg)
{
    if (!client_->initialize(SockAddr::FromString("127.0.0.1:12345")))
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
