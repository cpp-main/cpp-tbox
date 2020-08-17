#include "client.h"

#include <cassert>
#include <thread>
#include <tbox/base/log.h>

#include <mosquitto.h>

namespace tbox {
namespace mqtt {

using namespace std;
using namespace event;

struct Client::Data {
    Loop        *wp_loop = nullptr;

    Config      config;
    Callbacks   callback;

    mosquitto  *sp_mosq = nullptr;  //!< mosquitto 对象

    TimerEvent *sp_timer_ev      = nullptr;
    FdEvent    *sp_sock_read_ev  = nullptr;
    FdEvent    *sp_sock_write_ev = nullptr;

    //! 工作状态
    enum class State {
        kNone = 0,      //!< 未初始化
        kInited,        //!< 已初始化
        kConnecting,    //!< 正在连接
        kConnected,     //!< 已连接
    };
    State state;

    int cb_level = 0;   //! 回调层级
    bool is_connected = false;

    std::thread *sp_thread = nullptr;

    static int _instance_count;
};

int Client::Data::_instance_count = 0;

Client::Client(Loop *wp_loop) :
    d_(new Data)
{
    assert(d_ != nullptr);
    d_->wp_loop = wp_loop;

    if (Data::_instance_count == 0)
        mosquitto_lib_init();
    ++Data::_instance_count;
}

Client::~Client()
{
    assert(d_->cb_level == 0);
    delete d_;

    if (Data::_instance_count > 0) {
        --Data::_instance_count;
        if (Data::_instance_count == 0)
            mosquitto_lib_cleanup();
    }
}

bool Client::initialize(const Config &config, const Callbacks &callbacks)
{
    LogUndo();
    return false;
}

void Client::cleanup()
{
    LogUndo();
}

bool Client::start()
{
    LogUndo();
    return false;
}

bool Client::stop()
{
    LogUndo();
    return false;
}

int Client::subscribe(const std::string &topic, int *mid, int qos)
{
    LogUndo();
    return 0;
}

int Client::ubsubscribe(const std::string &topic, int *mid)
{
    LogUndo();
    return 0;
}

int Client::publish(const std::string &topic, const void *payload_ptr, size_t payload_size,
                    int qos, bool retain, int *mid)
{
    LogUndo();
    return 0;
}

bool Client::isConnected() const
{
    LogUndo();
    return false;
}

}
}
