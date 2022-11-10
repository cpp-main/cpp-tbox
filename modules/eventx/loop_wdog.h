#ifndef TBOX_LOOP_WDOG_H_20221110
#define TBOX_LOOP_WDOG_H_20221110

#include <string>
#include <functional>

namespace tbox {
namespace eventx {

/**
 * Loop看门狗
 */
class LoopWDog {
  public:
    using LoopDieCallback = std::function<void (const std::string&)>;

    //! 在 main() 中调用
    static void Start();    //! 启动线程监护
    static void Stop();     //! 停止线程监护

    //! (可选) 设置线程异常时的回调
    static void SetLoopDieCallback(const LoopDieCallback &cb);

    //! 由被监控的线程调用
    static void Register(event::Loop *loop, const std::string &loop_name);  //! 注册线程
    static void Unregister(event::Loop *loop);   //! 注销线程
};

}
}

#endif //TBOX_LOOP_WDOG_H_20221110
