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

    //! 基础设置
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

    //! TLS 设置
    if (d_->config.tls.enabled) {
        const char *ca_file   = nullptr;
        const char *ca_path   = nullptr;
        const char *cert_file = nullptr;
        const char *key_file  = nullptr;

        if (!d_->config.tls.ca_file.empty())
            ca_file = d_->config.tls.ca_file.c_str();

        if (!d_->config.tls.ca_path.empty())
            ca_path = d_->config.tls.ca_path.c_str();

        if (!d_->config.tls.cert_file.empty()) {
            cert_file = d_->config.tls.cert_file.c_str();
            key_file  = d_->config.tls.key_file.c_str();
        }

        //! TODO: 没有完善 pw_callback 功能，将来需要再实现，暂时填nullptr
        mosquitto_tls_set(d_->sp_mosq, ca_file, ca_path, cert_file, key_file, nullptr);

        const char *ssl_version = nullptr;
        const char *ciphers = nullptr;  //! TODO: 暂不设置

        if (!d_->config.tls.ssl_version.empty())
            ssl_version = d_->config.tls.ssl_version.c_str();

        mosquitto_tls_opts_set(d_->sp_mosq, d_->config.tls.is_require_peer_cert, ssl_version, ciphers);
        mosquitto_tls_insecure_set(d_->sp_mosq, d_->config.tls.is_insecure);
    }

    //! Will 设置
    if (d_->config.will.enabled) {
        mosquitto_will_set(d_->sp_mosq,
                           d_->config.will.topic.c_str(),
                           d_->config.will.payload.size(),
                           d_->config.will.payload.data(),
                           d_->config.will.qos,
                           d_->config.will.retain);
    }

    const char *username = nullptr;
    const char *passwd   = nullptr;

    if (!d_->config.base.username.empty())
        username = d_->config.base.username.c_str();

    if (!d_->config.base.passwd.empty())
        passwd = d_->config.base.passwd.c_str();

    if (username != nullptr)
        mosquitto_username_pw_set(d_->sp_mosq, username, passwd);

    d_->state = Data::State::kConnecting;

    CHECK_DELETE_RESET_OBJ(d_->sp_thread);
    //! 由于 mosquitto_connect() 是阻塞函数，为了避免阻塞其它事件，特交给子线程去做
    d_->sp_thread = new thread(
        [this] {
            int ret = mosquitto_connect(d_->sp_mosq,
                                        d_->config.base.broker.domain.c_str(),
                                        d_->config.base.broker.port,
                                        d_->config.base.keepalive);
            d_->wp_loop->runInLoop(
                [this, ret] {
                    onMosquittoConnectDone(ret);
                }
            );
        }
    );

    return true;
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

void Client::OnLogWrapper(struct mosquitto *, void *userdata, int level, const char *str)
{
    LogUndo();
}

void Client::onMosquittoConnectDone(int ret)
{
    d_->sp_thread->join();
    CHECK_DELETE_RESET_OBJ(d_->sp_thread);

    if (ret == MOSQ_ERR_SUCCESS) {
        enableSockeRead();
        enableSockeWriteIfNeed();
        d_->state = Data::State::kConnected;

    } else {
        LogWarn("connect fail, ret:%d", ret);
    }

    enableTimer();
}

void Client::enableSockeRead()
{
    if (mosquitto_socket(d_->sp_mosq) < 0)
        return;

    if (d_->sp_sock_read_ev->isEnabled())
        return;

    d_->sp_sock_read_ev->initialize(mosquitto_socket(d_->sp_mosq), FdEvent::kReadEvent, Event::Mode::kPersist);
    d_->sp_sock_read_ev->enable();
}

void Client::enableSockeWrite()
{
    if (mosquitto_socket(d_->sp_mosq) < 0)
        return;

    if (d_->sp_sock_write_ev->isEnabled())
        return;

    d_->sp_sock_write_ev->initialize(mosquitto_socket(d_->sp_mosq), FdEvent::kWriteEvent, Event::Mode::kOneshot);
    d_->sp_sock_write_ev->enable();
}

void Client::enableSockeWriteIfNeed()
{
    if (mosquitto_want_write(d_->sp_mosq))
        enableSockeWrite();
}

void Client::enableTimer()
{
    d_->sp_timer_ev->enable();
}

void Client::disableSockeRead()
{
    d_->sp_sock_read_ev->disable();
}

void Client::disableSockeWrite()
{
    d_->sp_sock_write_ev->disable();
}

void Client::disableTimer()
{
    d_->sp_timer_ev->disable();
}

}
}
