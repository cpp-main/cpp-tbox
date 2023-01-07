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

    enum class Mode {
        kAsync, //!< 异步
        kSync   //!< 同步
    };

    using MessageCallback = std::function<void(const Message &msg)>;
    using RequestCallback = std::function<void(const Request &req)>;

    //! 订阅消息
    Token subscribe(const std::string &topic, const MessageCallback &cb);
    //! 取消订阅
    bool unsubscribe(const Token &token);

    //! 发布消息
    void publish(const std::string &topic,          //!< 消息topic
                 const void *msg_ptr = nullptr,     //!< 消息内容地址
                 size_t msg_size = 0,               //!< 消息内容长度
                 Mode mode = Mode::kAsync);         //!< 模式：异步或同步


    //! 注册调用
    Token reg(const std::string &topic, const RequestCallback &cb);
    //! 注销调用
    bool unreg(const Token &token);

    //! 调用
    size_t invoke(const std::string &topic,         //!< 调用topic
                  const void *in_ptr = nullptr,     //!< 输入内容地址
                  void *out_ptr = nullptr) const;   //!< 输出内容地址

    //! 清理
    void cleanup();

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}
}

#endif //TBOX_BROKER_H_20220127
