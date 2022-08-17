#ifndef TBOX_UTIL_TIMEOUT_MONITOR_H_20220428
#define TBOX_UTIL_TIMEOUT_MONITOR_H_20220428

#include <tbox/base/cabinet_token.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace eventx {

/**
 * Token的超时监控器
 *
 * 该类通常配合cabinet::Cabinet使用，实现请求池功能
 * 用于管理请求的超时自动处理功能
 */
class TimeoutMonitor {
  public:
    using Token = cabinet::Token;
    using Duration = std::chrono::milliseconds;
    using Callback = std::function<void(const Token&)>;

  public:
    explicit TimeoutMonitor(event::Loop *wp_loop);
    virtual ~TimeoutMonitor();

    /**
     * \brief   初始化
     *
     * \param   check_interval  指定检查时间间隔
     * \param   check_times     指定检查次数
     * \param   timeout_action  指定超时的动作
     *
     * \return  bool    成功与否，通常都不会失败
     * \note    尽要权衡，不要让check_times太大
     */
    bool initialize(const Duration &check_interval, int check_times);
    void setCallback(const Callback &cb);

    void add(const Token &token);

    void cleanup();

  private:
    class Impl;
    Impl *impl_;
};

}
}

#endif //TBOX_UTIL_TIMEOUT_MONITOR_H_20220428
