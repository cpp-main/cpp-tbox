#ifndef TBOX_EVENTX_LOOP_THREAD_H_20230604
#define TBOX_EVENTX_LOOP_THREAD_H_20230604

#include <string>
#include <thread>

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace eventx {

/// 单独运行Loop的线程
class LoopThread {
  public:
    /**
     * \param run_now   是否立即运行，否则需要在后面调用start()启动
     * \param loop_name Loop名称，用于LoopWDog监测报警时打印显示
     */
    LoopThread(bool run_now = true,
               const std::string &loop_name = "loop_thread");

    ~LoopThread(); 

    NONCOPYABLE(LoopThread);
    IMMOVABLE(LoopThread);

  public:
    /// 启动线程
    void start();

    /// 停止线程
    void stop();

    /// 线程是否正在运行
    bool isRunning() const { return is_running_; }

    /// 返回 Loop 对象
    /**
     * WARN:
     * 1.不能在外部直接 delete Loop 对象
     * 2.在运行过程中，仅能调用runInLoop()与run()
     */
    event::Loop* loop() const { return loop_; };

  private:
    std::string name_;
    event::Loop *loop_;
    std::thread thread_;
    bool is_running_ = false;
};

}
}

#endif //TBOX_EVENTX_LOOP_THREAD_H_20230604
