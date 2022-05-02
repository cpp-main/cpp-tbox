#include "server.h"
#include "server_imp.h"

namespace tbox {
namespace http {

Server::Server(Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{ }

Server::~Server()
{
    delete impl_;
}

bool Server::initialize(const network::SockAddr &bind_addr, int listen_backlog)
{
    return impl_->initialize(bind_addr, listen_backlog);
}

bool Server::start()
{
    return impl_->start();
}

void Server::stop()
{
    impl_->stop();
}

void Server::cleanup()
{
    impl_->cleanup();
}

void Server::use(const RequestCallback &cb)
{
    impl_->use(cb);
}

void Server::use(Middleware *wp_middleware)
{
    impl_->use(wp_middleware);
}

}
}
