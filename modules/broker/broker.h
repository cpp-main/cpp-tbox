#ifndef TBOX_BROKER_H_20220127
#define TBOX_BROKER_H_20220127

#include <tbox/base/cabinet.hpp>
#include <tbox/event/loop.h>

namespace tbox {
namespace broker {

/**
 * 消息订阅与发布
 */
class Broker {
  public:
    using Token = cabinet::Token;

  public:
    explicit Broker(event::Loop *wp_loop);
    virtual ~Broker();

  public:
    //! 消息内容
    struct Message {
        std::string topic;      //! 主题
        void       *data_ptr;   //! 数据地址
        size_t      data_size;  //! 数据大小
    };

    //! 请求内容
    struct Request {
        std::string topic;      //! 主题
        const void *in_ptr;     //! 数据地址
        void       *out_ptr;    //! 数据地址
    };

    using MessageCallback = std::function<void(const Message &msg)>;
    using RequestCallback = std::function<void(const Request &req)>;

    //! 订阅
    Token subscribe(const std::string &topic, const MessageCallback &cb);
    //! 取消订阅
    bool unsubscribe(const Token &token);

    //! 发布消息
    void publish(const std::string &topic, const void *msg_ptr = nullptr, size_t msg_size = 0);


    //! 注册
    Token reg(const std::string &topic, const RequestCallback &cb);
    //! 注销
    bool unreg(const Token &token);

    //! 调用
    size_t invoke(const std::string &topic, const void *in_ptr = nullptr, void *out_ptr = nullptr) const;

    //! 清理
    void cleanup();

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}
}

#endif //TBOX_BROKER_H_20220127
