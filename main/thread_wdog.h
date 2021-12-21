/**
 * thread_wdog.h 线程看门狗
 *
 * 在进程中可能会同时开启多个线程工作。在运行过程中可能存在个别线程由于调用了阻塞函数、
 * 进入了死循环或其它原因阻塞了。我们需要一种机制能尽早发现该问题，并能及时报错与处理。
 *
 * ThreadWDog 就是类似于硬件看门狗机制的线程监控机制。
 *
 * 使用方法如下：
 *
 * 在main()进入时调用Start()启用线程看门狗模块，在退出时调用Stop()停止监控
 * int main()
 * {
 *     ThreadWDog::Start();
 *     ...
 *     ThreadWDog::Stop();
 * }
 *
 * 在子线程开始时调用 Register() 注册对自己的监控, 退出时调用 Unregister() 注销监控
 * 在子线程正常运行时定时调用 FeedDog() 方法喂狗
 *
 * void ThreadProc()
 * {
 *     ThreadWDog::Register("thread name", 30);
 *     while(true) {
 *         ...
 *         ThreadWDog::FeedDog();
 *         ...
 *     }
 *     ThreadWDog::Unregister();
 * }
 */
#ifndef TBOX_THREAD_WDOG_H_20211221
#define TBOX_THREAD_WDOG_H_20211221

#include <string>
#include <functional>

namespace tbox {
namespace main {

/**
 * 线程看门狗
 */
class ThreadWDog {
  public:
    using ThreadDieCallback = std::function<void (pid_t, const std::string&)>;

    //! 在 main() 中调用
    static void Start();    //! 启动线程监护
    static void Stop();     //! 停止线程监护

    //! (可选) 设置线程异常时的回调
    static void SetThreadDieCallback(const ThreadDieCallback &cb);

    //! 由被监控的线程调用
    static void Register(const std::string &thread_name, uint16_t timeout_sec);  //! 注册线程
    static void Unregister();   //! 注销线程
    static void FeedDog();      //! 喂狗
};

}
}

#endif //TBOX_THREAD_WDOG_H_20211221
