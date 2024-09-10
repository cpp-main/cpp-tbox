/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "client.h"

#include <thread>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/lifetime_tag.hpp>
#include <tbox/base/wrapped_recorder.h>
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
    int reconnect_wait_remain_sec = 0;  //! 重连等待剩余时长

    std::thread *sp_thread = nullptr;

    LifetimeTag alive_tag;  //!< 对像存活标签
    /// Q1: 为什么需要定义它？
    ///
    /// 因为在下面使用了runInLoop()函数注册了延后执行的回调函数。在该函数中将this指针捕获了。
    /// 在它真正执行的时候如果 this 指针所指向的对象就已经被 delete 了，那么如果我们还继续
    /// 访问那么就一定会触发访问失效的内存地址，行为不可预知。
    ///
    /// 怎么解决呢？
    /// 方法一：在对象析构的时候，撤消之前通过 runInLoop() 注册的回调函数；
    /// 方法二：在回调函数真正执行的时候，判定一下 this 所指的对象是否有效。
    ///
    /// 这里，我们采用了方法二。
    /// 那怎么实现在后期能知晓 this 指针是否有效呢？
    /// 通用的方案是：将 this 指针用 std::shared_ptr 进行管理。传入回调函数中的是它的 weak_ptr
    /// 这样，在使用 this 指针的时候就能得知它是否有效了。
    ///
    /// 这么做有一个缺点：这个对象必须继续于 enable_this_shared_ptr 类，而且实例化它的时候必须
    /// 使用 std::make_shared<T>() 才有效。这样种会给使用者造成一定的负担。
    /// 除此之外，还有没有别的方法？于是，我想到了 alive_tag。
    /// 由于对象的成员的生命期与对象自身是一致的，判定一个对象是否有效，可以通过判定它的某个成
    /// 员生命期是否有效即可。内部附带一个任意类型的智能指针。于是，我在这里定义了一个 alive_tag
    /// 的智能指针。它的类型可以是任意的，这里就选定了bool。
    /// 无所谓它的类型，反正我们也不会往里写内容。
    /// 在 runInLoop() 注册回调函数的时候，就通过 alive_tag 生成对应的 std::weak_ptr 并传入。
    /// 在回调函数执行之前，检查一下这个 std::weak_ptr 是否过期就可以得知 this 指针是否有效。
    /// 进而避免继续访问到无效 this 指向的内容。

    static int _instance_count;
};

int Client::Data::_instance_count = 0;

Client::Client(Loop *wp_loop) :
    d_(new Data)
{
    TBOX_ASSERT(d_ != nullptr);
    d_->wp_loop = wp_loop;

    if (Data::_instance_count == 0)
        mosquitto_lib_init();
    ++Data::_instance_count;

    d_->sp_mosq = mosquitto_new(nullptr, true, this);

    d_->sp_timer_ev = wp_loop->newTimerEvent("mqtt::Client::sp_timer_ev");
    d_->sp_timer_ev->initialize(chrono::seconds(1), Event::Mode::kPersist);
    d_->sp_timer_ev->setCallback(std::bind(&Client::onTimerTick, this));

    d_->sp_sock_read_ev  = wp_loop->newFdEvent("mqtt::Client::sp_sock_read_ev");
    d_->sp_sock_read_ev->setCallback(std::bind(&Client::onSocketRead, this));

    d_->sp_sock_write_ev = wp_loop->newFdEvent("mqtt::Client::sp_sock_write_ev");
    d_->sp_sock_write_ev->setCallback(std::bind(&Client::onSocketWrite, this));
}

Client::~Client()
{
    TBOX_ASSERT(d_->cb_level == 0);

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

    updateStateTo(State::kInited);
    return true;
}

void Client::cleanup()
{
    if (d_->state <= State::kNone)
        return;

    stop();

    d_->config = Config();
    d_->callbacks = Callbacks();
    updateStateTo(State::kNone);
}

bool Client::start()
{
    if (d_->state != State::kInited &&
        d_->state != State::kEnd) {
        LogWarn("state is not kInited or kEnd");
        return false;
    }

    RECORD_SCOPE();
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

    updateStateTo(State::kConnecting);

    CHECK_DELETE_RESET_OBJ(d_->sp_thread);
    auto is_alive = d_->alive_tag.get();  //! 原理见Q1

    //! 由于 mosquitto_connect() 是阻塞函数，为了避免阻塞其它事件，特交给子线程去做
    d_->sp_thread = new thread(
        [this, is_alive] {
            RECORD_SCOPE();
            int ret = mosquitto_connect_async(d_->sp_mosq,
                                              d_->config.base.broker.domain.c_str(),
                                              d_->config.base.broker.port,
                                              d_->config.base.keepalive);
            d_->wp_loop->runInLoop(
                [this, is_alive, ret] {
                    RECORD_SCOPE();
                    if (!is_alive) { //!< 判定this指针是否有效
                        LogWarn("object not alive");
                        return;
                    }

                    onTcpConnectDone(ret);
                    enableTimer();
                },
                "mqtt::Client::start, connect done"
            );
        }
    );

    return true;
}

void Client::stop()
{
    if (d_->state <= State::kInited ||
        d_->state == State::kEnd)
        return;

    RECORD_SCOPE();
    if (d_->state == State::kTcpConnected ||
        d_->state == State::kMqttConnected) {
        //! 如果已成功连接，则立即断开
        mosquitto_disconnect(d_->sp_mosq);  //! 请求断开，后续工作在 onDisconnected() 中处理
    } else {
        //! 如果正在连接，则停止连接线程
        if (d_->sp_thread != nullptr) {
            d_->sp_thread->join();
            CHECK_DELETE_RESET_OBJ(d_->sp_thread);
        }
    }

    disableTimer();

    updateStateTo(State::kInited);
    return;
}

int Client::subscribe(const std::string &topic, int *p_mid, int qos)
{
    if (d_->state != State::kMqttConnected) {
        LogWarn("mqtt is not connected");
        return false;
    }

    RECORD_SCOPE();
    int mid = 0;
    int ret = mosquitto_subscribe(d_->sp_mosq, &mid, topic.c_str(), qos);
    if (p_mid != nullptr)
        *p_mid = mid;

    enableSocketWriteIfNeed();
    return ret == MOSQ_ERR_SUCCESS;
}

int Client::unsubscribe(const std::string &topic, int *p_mid)
{
    if (d_->state != State::kMqttConnected) {
        LogWarn("mqtt is not connected");
        return false;
    }

    RECORD_SCOPE();
    int mid = 0;
    int ret = mosquitto_unsubscribe(d_->sp_mosq, &mid, topic.c_str());
    if (p_mid != nullptr)
        *p_mid = mid;

    enableSocketWriteIfNeed();
    return ret == MOSQ_ERR_SUCCESS;
}

int Client::publish(const std::string &topic, const void *payload_ptr, size_t payload_size,
                    int qos, bool retain, int *p_mid)
{
    if (d_->state != State::kMqttConnected) {
        LogWarn("mqtt is not connected");
        return false;
    }

    RECORD_SCOPE();
    int mid = 0;
    int ret = mosquitto_publish(d_->sp_mosq, &mid, topic.c_str(),
                                payload_size, payload_ptr,
                                qos, retain);
    if (p_mid != nullptr)
        *p_mid = mid;

    enableSocketWriteIfNeed();
    return ret == MOSQ_ERR_SUCCESS;
}

Client::State Client::getState() const
{
    return d_->state;
}

void Client::onTimerTick()
{
    if (d_->state == State::kTcpConnected ||
        d_->state == State::kMqttConnected) {
        mosquitto_loop_misc(d_->sp_mosq);

        if (mosquitto_socket(d_->sp_mosq) > 0)
            enableSocketWriteIfNeed();

    } else if (d_->state == State::kConnecting) {
        if (d_->sp_thread == nullptr) {
            auto is_alive = d_->alive_tag.get();  //! 原理见Q1
            d_->sp_thread = new thread(
                [this, is_alive] {
                    RECORD_SCOPE();
                    int ret = mosquitto_reconnect_async(d_->sp_mosq);
                    d_->wp_loop->runInLoop(
                        [this, is_alive, ret] {
                            RECORD_SCOPE();
                            if (!is_alive) {  //!< 判定this指针是否有效
                                LogWarn("object not alive");
                                return;
                            }
                            onTcpConnectDone(ret);
                        },
                        "mqtt::Client::onTimerTick, reconnect done"
                    );
                }
            );
        }

    } else if (d_->state == State::kReconnWaiting) {
        auto &remain_sec = d_->reconnect_wait_remain_sec;
        if (remain_sec > 0) {
            --remain_sec;
            if (remain_sec == 0) {
                updateStateTo(State::kConnecting);
                LogDbg("wait timeout, reconnect now");
            }
        }

    } else if (d_->state == State::kEnd) {
        disableTimer();
    }
}

void Client::onSocketRead()
{
    mosquitto_loop_read(d_->sp_mosq, 1);
    enableSocketWriteIfNeed();
}

void Client::onSocketWrite()
{
    mosquitto_loop_write(d_->sp_mosq, 1);
    enableSocketWriteIfNeed();
}

void Client::OnConnectWrapper(struct mosquitto *, void *userdata, int rc)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onConnected(rc);
}

void Client::OnDisconnectWrapper(struct mosquitto *, void *userdata, int rc)
{
    Client *p_this = static_cast<Client*>(userdata);
    p_this->onDisconnected(rc);
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

void Client::onConnected(int rc)
{
    if (rc != 0) {
        LogWarn("connect fail, rc:%d, %s", rc, mosquitto_strerror(rc));
        return;
    }

    LogInfo("connected");

    RECORD_SCOPE();
    if (d_->state != State::kMqttConnected) {
        updateStateTo(State::kMqttConnected);
        ++d_->cb_level;
        if (d_->callbacks.connected)
            d_->callbacks.connected();
        --d_->cb_level;
    }
}

void Client::onDisconnected(int rc)
{
    RECORD_SCOPE();
    LogNotice("disconnected, rc:%d, %s", rc, mosquitto_strerror(rc));
    handleDisconnectEvent();
}

void Client::onPublish(int mid)
{
    LogInfo("mid:%d", mid);

    RECORD_SCOPE();
    ++d_->cb_level;
    if (d_->callbacks.message_pub)
        d_->callbacks.message_pub(mid);
    --d_->cb_level;
}

void Client::onSubscribe(int mid, int qos, const int *granted_qos)
{
    LogInfo("mid:%d, qos:%d", mid, qos);

    RECORD_SCOPE();
    ++d_->cb_level;
    if (d_->callbacks.subscribed)
        d_->callbacks.subscribed(mid, qos, granted_qos);
    --d_->cb_level;
}

void Client::onUnsubscribe(int mid)
{
    LogInfo("mid:%d", mid);

    RECORD_SCOPE();
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

    RECORD_SCOPE();
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
        case MOSQ_LOG_ERR:
            new_level = LOG_LEVEL_ERROR;
            break;
        case MOSQ_LOG_WARNING:
            new_level = LOG_LEVEL_WARN;
            break;
        case MOSQ_LOG_NOTICE:
            new_level = LOG_LEVEL_NOTICE;
            break;
        default:; //! regard MOSQ_LOG_INFO as LOG_LEVEL_DEBUG
    }
    LogPrintfFunc("mosq", nullptr, nullptr, 0, new_level, 0, str);
}

void Client::onTcpConnectDone(int ret)
{
    RECORD_SCOPE();
    if (d_->sp_thread == nullptr) {
        LogWarn("sp_thread == nullptr");
        return;
    }

    d_->sp_thread->join();
    CHECK_DELETE_RESET_OBJ(d_->sp_thread);

    if (ret == MOSQ_ERR_SUCCESS) {
        LogDbg("connect success");
        enableSocketRead();
        enableSocketWriteIfNeed();
        updateStateTo(State::kTcpConnected);

    } else {
        LogNotice("connect fail, rc:%d, %s", ret, mosquitto_strerror(ret));
        tryReconnect();

        ++d_->cb_level;
        if (d_->callbacks.connect_fail)
            d_->callbacks.connect_fail();
        --d_->cb_level;
    }
}

void Client::enableSocketRead()
{
    if (mosquitto_socket(d_->sp_mosq) < 0)
        return;

    if (d_->sp_sock_read_ev->isEnabled())
        return;

    d_->sp_sock_read_ev->initialize(mosquitto_socket(d_->sp_mosq), FdEvent::kReadEvent, Event::Mode::kPersist);
    d_->sp_sock_read_ev->enable();
}

void Client::enableSocketWrite()
{
    if (mosquitto_socket(d_->sp_mosq) < 0)
        return;

    if (d_->sp_sock_write_ev->isEnabled())
        return;

    d_->sp_sock_write_ev->initialize(mosquitto_socket(d_->sp_mosq), FdEvent::kWriteEvent, Event::Mode::kOneshot);
    d_->sp_sock_write_ev->enable();
}

void Client::enableSocketWriteIfNeed()
{
    if (mosquitto_want_write(d_->sp_mosq))
        enableSocketWrite();
}

void Client::enableTimer()
{
    d_->sp_timer_ev->enable();
}

void Client::disableSocketRead()
{
    d_->sp_sock_read_ev->disable();
}

void Client::disableSocketWrite()
{
    d_->sp_sock_write_ev->disable();
}

void Client::disableTimer()
{
    d_->sp_timer_ev->disable();
}

void Client::tryReconnect()
{
    //! 如果开启了自动重连
    if (d_->config.auto_reconnect_enable) {
        if (d_->config.auto_reconnect_wait_sec > 0) {
            LogDbg("reconnect after %d sec", d_->config.auto_reconnect_wait_sec);
            d_->reconnect_wait_remain_sec = d_->config.auto_reconnect_wait_sec;
            updateStateTo(State::kReconnWaiting);

        } else {
            LogDbg("reconnect now");
            d_->reconnect_wait_remain_sec = 0;
            updateStateTo(State::kConnecting);
        }

    } else {  //! 如果不需要自动重连
        LogDbg("no need reconnect, end");
        updateStateTo(State::kEnd);
    }
}

void Client::handleDisconnectEvent()
{
    disableSocketRead();
    disableSocketWrite();

    if (d_->state == State::kTcpConnected ||
        d_->state == State::kMqttConnected) {
        auto is_mqtt_disconnected = d_->state == State::kMqttConnected;
        //! 一定要先判定，因为 tryReconnect() 会改 d_->state

        tryReconnect();

        ++d_->cb_level;
        if (is_mqtt_disconnected) {
            if (d_->callbacks.disconnected)
                d_->callbacks.disconnected();
        } else {
            if (d_->callbacks.connect_fail)
                d_->callbacks.connect_fail();
        }
        --d_->cb_level;
    }
}

void Client::updateStateTo(State new_state)
{
    d_->state = new_state;
    LogDbg("new_state: %d", d_->state);

    ++d_->cb_level;
    if (d_->callbacks.state_changed)
        d_->callbacks.state_changed(new_state);
    --d_->cb_level;
}

}
}
