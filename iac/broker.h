#ifndef TBOX_IAC_BROKER_H_20220127
#define TBOX_IAC_BROKER_H_20220127

#include <tbox/base/cabinet.hpp>
#include <tbox/event/loop.h>

namespace tbox::iac {

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
        std::string topic;  //! 主题
        void  *data_ptr;    //! 数据地址
        size_t data_size;   //! 数据大小
    };
    using Callback = std::function<void(const Message &msg, const Token &token)>;

    //! 订阅
    Token subscribe(const std::string &topic, const Callback &cb);
    //! 取消订阅
    bool unsubscribe(const Token &token);

    //! 发布消息
    void publish(const std::string &topic, const void *msg_ptr = nullptr, size_t msg_size = 0);

    //! 清理
    void cleanup();

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}

#endif //TBOX_IAC_BROKER_H_20220127
