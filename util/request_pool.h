#ifndef TBOX_UTIL_REQUEST_POOL_H_20220427
#define TBOX_UTIL_REQUEST_POOL_H_20220427

#include <tbox/base/cabinet_token.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace util {

/**
 * 请求池
 *
 * 在服务型异步开发时，我们会面临客户端的请求。而有些请求又没办法立即完成，比如说：请写文件、发送另一个请求。
 * 客户端在请求的时候很可能需要我们带一些数据回去，比如：请求号、时间戳、用户ID等。
 * 最原始的做法：将这里业务不相关的数据，一路都带上，从头到尾走完整个业务处理流程。
 * 然而，这样会有两个问题：
 * - 效率问题，业务处理的过程一路带上，会有附加的传参；
 * - 扩展性问题，如果客户端又附加了其它数据，都要在整个流程中都要加；
 *
 * 那有没有更好的解决方案呢？
 * 就是本模块：请求池
 * 它有以下功能：
 * 1.能存储客户端请求中一些业务处理不需要关心的数据，生成一个token；
 * 2.在超时之后，能自动回复一个超时数据包；
 * 3.在业务处理完后，可根据token获取请求的上下文。
 */
class RequestPool {
  public:
    using Token = cabinet::Token;
    using Seconds = std::chrono::milliseconds;
    using TimeoutAction = std::function<void(void*)>;

  public:
    explicit RequestPool(event::Loop *wp_loop);
    virtual ~RequestPool();

    /**
     * \brief   初始化
     *
     * \param   timeout_sec     指定超时时长，单位：秒
     * \param   timeout_action  指定超时的动作
     *
     * \return  bool    成功与否，通常都不会失败
     *
     * \note    请求上下文指针指向的数据需要由用户自己去释放，RequestPool不负责其生命期
     */
    bool initialize(const Seconds &timeout_sec, const TimeoutAction &timeout_action);
    bool start();
    void stop();
    void cleanup();

    /**
     * \brief   新建一个请求
     *
     * \param   req_ctx 请求的上下文件
     *
     * \return  Token   请求的标识，后面可以拿这个标识来取之前存入的req_ctx
     */
    Token newRequest(void *req_ctx = nullptr);

    /**
     * \brief   更新请求的上下文
     *
     * \param   token   请求的标识
     * \param   req_ctx 请求的上下文
     *
     * \return  bool    成功与否，如果返回false表示token所指的请求不存在
     *
     * \note    通常用于 newRequest() 时不方法提供上下文件，后面又需要更新的场景
     */
    bool updateRequest(const Token &token, void *req_ctx);

    /**
     * \brief   根据token取走请求的上下文
     *
     * \param   token   请求的标识
     *
     * \return  void*   请求上下文地扯
     *
     * \note    在移除之后，RequestPool中就不再有该请求的记录了
     */
    void* removeRequest(const Token &token);

  private:
    class Impl;
    Impl *impl_;
};

}
}

#endif //TBOX_UTIL_REQUEST_POOL_H_20220427
