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

    State state = State::kNone;

    int cb_level = 0;   //! 回调层级

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
    if (d_->state != State::kNone) {
        LogWarn("cleanup first");
        return false;
    }

    if (!config.isValid())
        return false;

    d_->config = config;
    d_->callbacks = callbacks;

    d_->state = State::kInited;
    return true;
}

void Client::cleanup()
{
    if (d_->state <= State::kNone)
        return;

    stop();

    d_->config = Config();
    d_->callbacks = Callbacks();
    d_->state = State::kNone;
}

bool Client::start()
{
    if (d_->state != State::kInited) {
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
        mosquitto_message_callback_set(d_->sp_mosq, OnMessageWrapper);
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

    d_->state = State::kConnecting;

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
                    onMosquittoConnectDone(ret, true);
                }
            );
        }
    );

    return true;
}

void Client::stop()
{
    if (d_->state <= State::kInited)
        return;

    if (d_->state == State::kConnected) {
        //! 如果已成功连接，则立即断开
        mosquitto_disconnect(d_->sp_mosq);  //! 请求断开，后续工作在 onDisconnect() 中处理
    } else {
        //! 如果正在连接，则停止连接线程
        if (d_->sp_thread != nullptr) {
            d_->sp_thread->join();
            CHECK_DELETE_RESET_OBJ(d_->sp_thread);
        }
    }

    disableTimer();

    d_->state = State::kInited;
    return;
}

int Client::subscribe(const std::string &topic, int *p_mid, int qos)
{
    if (d_->state != State::kConnected) {
        LogWarn("broke is disconnected");
        return false;
    }

    int mid = 0;
    int ret = mosquitto_subscribe(d_->sp_mosq, &mid, topic.c_str(), qos);
    if (p_mid != nullptr)
        *p_mid = mid;

    enableSockeWriteIfNeed();
    return ret == MOSQ_ERR_SUCCESS;
}

int Client::ubsubscribe(const std::string &topic, int *p_mid)
{
    if (d_->state != State::kConnected) {
        LogWarn("broke is disconnected");
        return false;
    }

    int mid = 0;
    int ret = mosquitto_unsubscribe(d_->sp_mosq, &mid, topic.c_str());
    if (p_mid != nullptr)
        *p_mid = mid;

    enableSockeWriteIfNeed();
    return ret == MOSQ_ERR_SUCCESS;
}

int Client::publish(const std::string &topic, const void *payload_ptr, size_t payload_size,
                    int qos, bool retain, int *p_mid)
{
    if (d_->state != State::kConnected) {
        LogWarn("broke is disconnected");
        return false;
    }

    int mid = 0;
    int ret = mosquitto_publish(d_->sp_mosq, &mid, topic.c_str(),
                                payload_size, payload_ptr,
                                qos, retain);
    if (p_mid != nullptr)
        *p_mid = mid;

    enableSockeWriteIfNeed();
    return ret == MOSQ_ERR_SUCCESS;
}

Client::State Client::getState() const
{
    return d_->state;
}

void Client::onTimerTick()
{
    if (d_->state == State::kConnected) {
        mosquitto_loop_misc(d_->sp_mosq);

        if (mosquitto_socket(d_->sp_mosq) < 0) {
            LogInfo("disconnected with broker, retry.");
            d_->state = State::kConnecting;
        } else {
            enableSockeWriteIfNeed();
        }
    }

    if (d_->state == State::kConnecting) {
        if (d_->sp_thread == nullptr) {
            d_->sp_thread = new thread(
                [this] {
                    int ret = mosquitto_reconnect(d_->sp_mosq);
                    d_->wp_loop->runInLoop(
                        [this, ret] {
                            onMosquittoConnectDone(ret, false);
                        }
                    );
                }
            );
        }
    }
}

void Client::onSocketRead()
{
    mosquitto_loop_read(d_->sp_mosq, 1);
    enableSockeWriteIfNeed();
}

void Client::onSocketWrite()
{
    mosquitto_loop_write(d_->sp_mosq, 1);
    enableSockeWriteIfNeed();
}

void Client::OnConnectWrapper(struct mosquitto *, void *userdata, int rc)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onConnect(rc);
}

void Client::OnDisconnectWrapper(struct mosquitto *, void *userdata, int rc)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onDisconnect(rc);
}

void Client::OnPublishWrapper(struct mosquitto *, void *userdata, int mid)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onPublish(mid);
}

void Client::OnMessageWrapper(struct mosquitto *, void *userdata, const struct mosquitto_message *msg)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onMessage(msg);
}

void Client::OnSubscribeWrapper(struct mosquitto *, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onSubscribe(mid, qos_count, granted_qos);
}

void Client::OnUnsubscribeWrapper(struct mosquitto *, void *userdata, int mid)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onUnsubscribe(mid);
}

void Client::OnLogWrapper(struct mosquitto *, void *userdata, int level, const char *str)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onLog(level, str);
}

void Client::onConnect(int rc)
{
    if (rc != 0) {
        LogWarn("connect fail, rc:%d", rc);
        return;
    }

    LogInfo("connected");

    ++d_->cb_level;
    if (d_->callbacks.connected)
        d_->callbacks.connected();
    --d_->cb_level;
}

void Client::onDisconnect(int rc)
{
    disableSockeRead();
    disableSockeWrite();

    LogInfo("disconnected");

    ++d_->cb_level;
    if (d_->callbacks.disconnected)
        d_->callbacks.disconnected();
    --d_->cb_level;
}

void Client::onPublish(int mid)
{
    LogInfo("mid:%d", mid);

    ++d_->cb_level;
    if (d_->callbacks.message_pub)
        d_->callbacks.message_pub(mid);
    --d_->cb_level;
}

void Client::onSubscribe(int mid, int qos, const int *granted_qos)
{
    LogInfo("mid:%d, qos:%d", mid, qos);

    ++d_->cb_level;
    if (d_->callbacks.subscribed)
        d_->callbacks.subscribed(mid, qos, granted_qos);
    --d_->cb_level;
}

void Client::onUnsubscribe(int mid)
{
    LogInfo("mid:%d", mid);

    ++d_->cb_level;
    if (d_->callbacks.unsubscribed)
        d_->callbacks.unsubscribed(mid);
    --d_->cb_level;
}

void Client::onMessage(const struct mosquitto_message *msg)
{
    if (msg == nullptr)
        return;

    LogInfo("mid:%d, topic:%s", msg->mid, msg->topic);

    ++d_->cb_level;
    if (d_->callbacks.message_recv)
        d_->callbacks.message_recv(msg->mid,
                                   msg->topic,
                                   msg->payload,
                                   msg->payloadlen,
                                   msg->qos,
                                   msg->retain);
    --d_->cb_level;
}

void Client::onLog(int level, const char *str)
{
    auto new_level = LOG_LEVEL_DEBUG;
    switch (level & 0x1F) {
        case MOSQ_LOG_INFO:
            new_level = LOG_LEVEL_INFO;
            break;
        case MOSQ_LOG_ERR:
            new_level = LOG_LEVEL_ERROR;
            break;
        case MOSQ_LOG_WARNING:
            new_level = LOG_LEVEL_WARN;
            break;
        case MOSQ_LOG_NOTICE:
            new_level = LOG_LEVEL_NOTICE;
            break;
    }
    LogPrintfFunc("mosq", nullptr, nullptr, 0, new_level, "%s", str);
}

void Client::onMosquittoConnectDone(int ret, bool first_connect)
{
    d_->sp_thread->join();
    CHECK_DELETE_RESET_OBJ(d_->sp_thread);

    if (ret == MOSQ_ERR_SUCCESS) {
        enableSockeRead();
        enableSockeWriteIfNeed();
        d_->state = State::kConnected;
    } else {
        LogWarn("connect fail, ret:%d", ret);
    }

    //! 如果是首次连接要启动定时器，重连的不需要
    if (first_connect)
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
