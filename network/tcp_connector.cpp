#include "tcp_connector.h"

#include <cassert>

namespace tbox {
namespace network {

TcpConnector::TcpConnector(event::Loop *wp_loop) :
    wp_loop_(wp_loop)
{ }

TcpConnector::~TcpConnector()
{
    assert(cb_level_ == 0);

    CHECK_DELETE_RESET_OBJ(sp_delay_ev_);
    CHECK_DELETE_RESET_OBJ(sp_write_ev_);
}

void TcpConnector::checkSettingAndTryEnterIdleState()
{
    if (state_ == State::kNone) {
        if (server_addr_.type() != SockAddr::Type::kNone && connected_cb_)
            state_ = State::kIdle;
    }
}

void TcpConnector::setServer(const SockAddr &server_addr)
{
    server_addr_ = server_addr;
    checkSettingAndTryEnterIdleState();
}

void TcpConnector::setConnectedCallback(const ConnectedCallback &cb)
{
    connected_cb_ = cb;
    checkSettingAndTryEnterIdleState();
}

void TcpConnector::setTryTimes(int try_times)
{
    try_times_ = try_times;
}

void TcpConnector::setConnectFailCallback(const ConnectFailCallback &cb)
{
    connect_fail_cb_ = cb;
}

void TcpConnector::setReconnectDelayCalcFunc(const ReconnectDelayCalc &func)
{
    reconn_delay_calc_func_ = func;
}

bool TcpConnector::start()
{
    if (state_ != kIdle) {
        LogWarn("not in idle state");
        return false;
    }

    conn_fail_times_ = 0;
    enterConnectingState();
    return true;
}

bool TcpConnector::stop()
{
    if ((state_ == State::kConnecting) || (state_ == State::kNone))
        return false;

    if (state_ == State::kConnecting)
        exitConnectingState();
    else if (state_ == State::kReconnectDelay)
        exitReconnectDelayState();

    state_ = State::kIdle;
    return true;
}

}
}
