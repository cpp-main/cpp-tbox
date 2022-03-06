#include "tcp_rpc.h"
#include "../impl/service/tcp_rpc.h"

namespace tbox {
namespace terminal {

TcpRpc::TcpRpc(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    impl_(new Impl(wp_loop, wp_terminal))
{
    assert(impl_ != nullptr);
}

TcpRpc::~TcpRpc()
{
    delete impl_;
}

bool TcpRpc::initialize(const std::string &bind_addr)
{
    return impl_->initialize(bind_addr);
}

bool TcpRpc::start()
{
    return impl_->start();
}

void TcpRpc::stop()
{
    return impl_->stop();
}

void TcpRpc::cleanup()
{
    return impl_->cleanup();
}

}
}
