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
#ifndef TBOX_MQTT_CLIENT_H_20200815
#define TBOX_MQTT_CLIENT_H_20200815

#include <tbox/base/defines.h>
#include <tbox/base/memblock.h>
#include <tbox/event/loop.h>

struct mosquitto;
struct mosquitto_message;

namespace tbox {
namespace mqtt {

class Client {
  public:
    explicit Client(event::Loop *wp_loop);
    virtual ~Client();

    NONCOPYABLE(Client);
    IMMOVABLE(Client);

  public:
    //! 状态
    enum class State {
        kNone = 0,      //!< 未初始化
        kInited,        //!< 已初始化
        kConnecting,    //!< 正在连接
        kTcpConnected,  //!< TCP已连接
        kMqttConnected, //!< MQTT已连接
        kReconnWaiting, //!< 断连等待中
        kEnd,           //!< 终止，断连后又不需要重连的情况
    };

    //! 配置项
    struct Config {
        //! 基础配置
        struct Base {
            struct Broker {
                std::string domain = "localhost";   //! 地址
                uint16_t port = 1883;               //! 端口
            };
            Broker broker;                      //! Broker信息
            std::string client_id;              //! 客户端ID
            std::string username, passwd;       //! 用户名与密码
            int keepalive = 60;                 //! 心跳时长（秒）
        };
        //! 加密配置
        struct TLS {
            bool enabled = false;               //! 是否启用
            std::string ca_file, ca_path;       //! CA文件或路径，二选一
            std::string cert_file, key_file;    //! 证书与密钥，如果有则一起有
            bool is_require_peer_cert = true;   //! 是否需要验对方的证书
            std::string ssl_version;            //! SSL版本
            bool is_insecure = false;           //! 是否非安全使用
        };
        //! 遗言配置
        struct Will {
            bool enabled = false;               //! 是否启动遗言
            std::string topic;                  //! 遗言Topic
            Memblock payload;                   //! 遗言内容
            int qos = 0;
            bool retain = false;
        };

        Base base;
        Will will;
        TLS  tls;

        bool auto_reconnect_enable = true;   //! 是否自动重连
        int  auto_reconnect_wait_sec = 0;    //! 自动重连等待时长，秒

        bool isValid() const;
    };

    //! 回调函数类型定义
    using ConnectedCallback         = std::function<void()>;
    using ConnectFailCallback       = std::function<void()>;
    using DisconnectedCallback      = std::function<void()>;
    using MessageReceivedCallback   = std::function<void(int /*mid*/, const std::string &/*topic*/,
                                                         const void* /*payload_ptr*/, int/*payload_size*/,
                                                         int /*qos*/, bool /*retain*/)>;
    using MessagePublishedCallback  = std::function<void(int /*mid*/)>;
    using TopicSubscribedCallback   = std::function<void(int /*mid*/, int /*qos*/, const int* /*granted_qos*/)>;
    using TopicUnsubscribedCallback = std::function<void(int /*mid*/)>;
    using StateChangedCallback      = std::function<void(State)>;

    //! 回调函数
    struct Callbacks {
        ConnectedCallback           connected;
        ConnectFailCallback         connect_fail;
        DisconnectedCallback        disconnected;
        MessageReceivedCallback     message_recv;
        MessagePublishedCallback    message_pub;
        TopicSubscribedCallback     subscribed;
        TopicUnsubscribedCallback   unsubscribed;
        StateChangedCallback        state_changed;
    };

    bool initialize(const Config &config, const Callbacks &callbacks);
    void cleanup();

    bool start();
    void stop();

    //! 订阅与取消订阅
    int subscribe(const std::string &topic, int *mid = nullptr, int qos = 0);
    int unsubscribe(const std::string &topic, int *mid = nullptr);

    //! 发布消息
    int publish(const std::string &topic,
                const void *payload_ptr = nullptr, size_t payload_size = 0,
                int qos = 0, bool retain = false, int *mid = nullptr);

    State getState() const;

  protected:
    void onTimerTick();
    void onSocketRead();
    void onSocketWrite();

    static void OnConnectWrapper(struct mosquitto *, void *userdata, int rc);
    static void OnDisconnectWrapper(struct mosquitto *, void *userdata, int rc);
    static void OnPublishWrapper(struct mosquitto *, void *userdata, int mid);
    static void OnMessageWrapper(struct mosquitto *, void *userdata, const struct mosquitto_message *msg);
    static void OnSubscribeWrapper(struct mosquitto *, void *userdata, int mid, int qos_count, const int *granted_qos);
    static void OnUnsubscribeWrapper(struct mosquitto *, void *userdata, int mid);
    static void OnLogWrapper(struct mosquitto *, void *userdata, int level, const char *str);

    void onConnected(int rc);
    void onDisconnected(int rc);
    void onPublish(int mid);
    void onSubscribe(int mid, int qos, const int *granted_qos);
    void onUnsubscribe(int mid);
    void onMessage(const struct mosquitto_message *msg);
    void onLog(int level, const char *str);

    void onTcpConnectDone(int ret);

    void enableSocketRead();
    void enableSocketWrite();
    void enableSocketWriteIfNeed();
    void enableTimer();

    void disableSocketRead();
    void disableSocketWrite();
    void disableTimer();

    void tryReconnect();
    void handleDisconnectEvent();
    void updateStateTo(State new_state);

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_MQTT_CLIENT_H_20200815
