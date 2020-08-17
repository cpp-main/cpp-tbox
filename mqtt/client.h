#ifndef TBOX_MQTT_CLIENT_H_20200815
#define TBOX_MQTT_CLIENT_H_20200815

#include <tbox/base/defines.h>
#include <tbox/base/memblock.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace mqtt {

class Client {
  public:
    explicit Client(event::Loop *wp_loop);
    virtual ~Client();

    NONCOPYABLE(Client);
    IMMOVABLE(Client);

  public:
    //! 配置项
    struct Config {
        struct Basic {
            struct Broker {
                std::string domain = "localhost";
                uint16_t port = 1883;
            };
            Broker broker;
            std::string client_id;
            std::string username, passwd;
            int keepalive = 60;
        };
        struct TLS {
            std::string ca_file, ca_path;
            std::string cert_file;
            std::string key_file;
            bool is_require_peer_cert = true;
            std::string ssl_version;
            bool is_insecure = false;
        };
        struct Will {
            std::string topic;
            Memblock payload;
            int qos = 0;
            bool retain = false;
        };

        Basic basic;
        Will  will;
        TLS   tls;
    };

    bool initialize(const Config &config);
    void cleanup();

    using ConnectedCallback         = std::function<void()>;
    using DisconnectedCallback      = std::function<void()>;
    using MessageReceivedCallback   = std::function<void(int /*mid*/, const std::string &/*topic*/,
                                                         const void* /*payload_ptr*/, int/*payload_size*/,
                                                         int /*qos*/, bool /*retain*/)>;
    using MessagePublishedCallback  = std::function<void(int /*mid*/)>;
    using TopicSubscribedCallback   = std::function<void(int /*mid*/, int /*qos*/, const int* /*granted_qos*/)>;
    using TopicUnsubscribedCallback = std::function<void(int /*mid*/)>;

    //! 回调函数
    struct Callbacks {
        ConnectedCallback           connected;
        DisconnectedCallback        disconnected;
        MessageReceivedCallback     message_recv;
        MessagePublishedCallback    message_pub;
        TopicSubscribedCallback     subscribed;
        TopicUnsubscribedCallback   unsubscribed;
    };

    void setCallbacks(const Callbacks &cbs);

    bool start();
    bool stop();

    int subscribe(const std::string &topic, int *mid = nullptr, int qos = 0);
    int ubsubscribe(const std::string &topic, int *mid = nullptr);
    int publish(const std::string &topic,
                const void *payload_ptr = nullptr, size_t payload_size = 0,
                int qos = 0, bool retain = false, int *mid = nullptr);

    bool isConnected() const;

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_MQTT_CLIENT_H_20200815
