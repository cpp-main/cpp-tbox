#ifndef TBOX_IAC_SERVICE_H_20220127
#define TBOX_IAC_SERVICE_H_20220127

#include <tbox/base/cabinet.hpp>
#include <tbox/event/loop.h>

namespace tbox::iac {

/**
 * 消息订阅与发布
 */
class Service {
  public:
    using Token = cabinet::Token;

  public:
    explicit Service(event::Loop *wp_loop);
    virtual ~Service();

  public:
    //! 正文
    struct Context {
        std::string topic;      //! 主题
        const void *in_ptr;     //! 数据地址
        void       *out_ptr;    //! 数据地址
    };

    using Callback = std::function<void(const Context &ctx, const Token &token)>;

    //! 注册
    Token reg(const std::string &topic, const Callback &cb);
    //! 注销
    bool unreg(const Token &token);

    //! 调用
    bool invoke(const std::string &topic, const void *in_ptr = nullptr, void *out_ptr = nullptr) const;
    //! 检查是否存在指定topic的处理
    bool isExist(const std::string &topic) const;

    //! 清理
    void cleanup();

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}

#endif //TBOX_IAC_SERVICE_H_20220127
