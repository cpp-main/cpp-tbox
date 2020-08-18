#include "client.h"

#include <cassert>
#include <thread>

#include <tbox/base/log.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/fd_event.h>

#include <mosquitto.h>

namespace tbox {
namespace mqtt {

using namespace std;
using namespace event;

struct Client::Data {
    Loop        *wp_loop = nullptr;

    Config      config;
    Callbacks   callbacks;

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
    State state = State::kNone;

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

    d_->sp_mosq = mosquitto_new(nullptr, true, this);

    d_->sp_timer_ev = wp_loop->newTimerEvent();
    d_->sp_timer_ev->initialize(chrono::seconds(1), Event::Mode::kPersist);
    d_->sp_timer_ev->setCallback(std::bind(&Client::onTimerTick, this));

    d_->sp_sock_read_ev  = wp_loop->newFdEvent();
    d_->sp_sock_read_ev->setCallback(std::bind(&Client::onSocketRead, this));

    d_->sp_sock_write_ev = wp_loop->newFdEvent();
    d_->sp_sock_write_ev->setCallback(std::bind(&Client::onSocketWrite, this));
}

Client::~Client()
{
    assert(d_->cb_level == 0);

    cleanup();

    delete d_->sp_sock_write_ev;
    delete d_->sp_sock_read_ev;
    delete d_->sp_timer_ev;
    mosquitto_destroy(d_->sp_mosq);
    delete d_;

    if (Data::_instance_count > 0) {
        --Data::_instance_count;
        if (Data::_instance_count == 0)
            mosquitto_lib_cleanup();
    }
}

bool Client::Config::isValid() const
{
    //! 检查base
    if (base.broker.domain.empty()) {
        LogWarn("broker domain invalid");
        return false;
    }

    if (base.broker.port == 0) {
        LogWarn("broker port == 0");
        return false;
    }

    if (base.keepalive < 0) {
        LogWarn("keeplive < 0");
        return false;
    }

    //! 检查tls
    if (tls.enabled) {
        if (tls.is_require_peer_cert) {
            //! 要验对方的证书，就必须得有CA。要么指定了文件，要么指定了路径
            if (tls.ca_file.empty() && tls.ca_path.empty()) {
                LogWarn("both CA file and path is empty");
                return false;
            }
        }

        if (!tls.cert_file.empty() && tls.key_file.empty()) {
            //! 如果指定了自己证书，但没有给出私钥，也是错的
            LogWarn("key_file is empty");
            return false;
        }
    }

    //! 检查will
    if (will.enabled) {
        if (will.topic.empty()) {
            LogWarn("will topic is empty");
            return false;
        }
    }

    return true;
}

bool Client::initialize(const Config &config, const Callbacks &callbacks)
{
    if (d_->state != Data::State::kNone) {
        LogWarn("cleanup first");
        return false;
    }

    if (!config.isValid())
        return false;

    d_->config = config;
    d_->callbacks = callbacks;

    d_->state = Data::State::kInited;
    return true;
}

void Client::cleanup()
{
    if (d_->state <= Data::State::kNone)
        return;

    stop();

    d_->config = Config();
    d_->callbacks = Callbacks();
    d_->state = Data::State::kNone;
}

bool Client::start()
{
    if (d_->state != Data::State::kInited) {
        LogWarn("state != kInited");
        return false;
    }

    const char *client_id = nullptr;
    if (!d_->config.base.client_id.empty())
        client_id = d_->config.base.client_id.c_str();
    int ret = mosquitto_reinitialise(d_->sp_mosq, client_id, true, this);
    if (ret == MOSQ_ERR_SUCCESS) {
        mosquitto_connect_callback_set(d_->sp_mosq, OnConnectWrapper);
        mosquitto_disconnect_callback_set(d_->sp_mosq, OnDisconnectWrapper);
        mosquitto_publish_callback_set(d_->sp_mosq, OnPublishWrapper);
        mosquitto_subscribe_callback_set(d_->sp_mosq, OnSubscribeWrapper);
        mosquitto_unsubscribe_callback_set(d_->sp_mosq, OnUnsubscribeWrapper);
        mosquitto_log_callback_set(d_->sp_mosq, OnLogWrapper);
    } else {
        LogWarn("mosquitto_reinitialise() fail, ret:%s, %s", ret, mosquitto_strerror(ret));
        return false;
    }

    //!TODO
    LogUndo();
    return false;
}

bool Client::stop()
{
    if (d_->state <= Data::State::kInited)
        return true;

    LogUndo();
    d_->state = Data::State::kInited;
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

void Client::onTimerTick()
{
    LogUndo();
}

void Client::onSocketRead()
{
    LogUndo();
}

void Client::onSocketWrite()
{
    LogUndo();
}

void Client::OnConnectWrapper(struct mosquitto *, void *userdata, int rc)
{
    LogUndo();
}

void Client::OnDisconnectWrapper(struct mosquitto *, void *userdata, int rc)
{
    LogUndo();
}

void Client::OnPublishWrapper(struct mosquitto *, void *userdata, int mid)
{
    LogUndo();
}

void Client::OnMessageWrapper(struct mosquitto *, void *userdata, const struct mosquitto_message *msg)
{
    LogUndo();
}

void Client::OnSubscribeWrapper(struct mosquitto *, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    LogUndo();
}

void Client::OnUnsubscribeWrapper(struct mosquitto *, void *userdata, int mid)
{
    LogUndo();
}
void OnLogWrapper(struct mosquitto *, void *userdata, int level, const char *str);

}
}
