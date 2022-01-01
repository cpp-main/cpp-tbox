#ifndef TBOX_MAIN_APP_H_20211222
#define TBOX_MAIN_APP_H_20211222

#include <tbox/base/json_fwd.h>

namespace tbox::main {

/**
 * 应用接口类
 *
 * 应用的生命期遵循以下几个过程：
 *
 *   构造 --> 初始化 --> 启动 --.
 *   析构 <--  清理  <-- 停止 <-'
 *
 * 以使用一台电脑为例：
 * 1）构造，将显示器、主机、键盘、鼠标等设备都逐一布置好；
 * 2）初始化，插好电源，将设备之间的线都连接上；
 * 3）启动，启动各个设备；
 * ... 系统正常工作 ...
 * 4）停止，关闭各个设备；
 * 5）清理，断开设备之间的连接线；
 * 6）析构，将设备逐一撤走；
 *
 * 对应App各步骤的动作：
 * 1）构造，构建应用相关的对象
 * 2）initialize，访问外部资源（读取文件）、建立对象之间的连接
 * 3）start，启动所有对象，令它们开始工作
 * 4）stop， 停止所有对象，令它们停止工作
 * 5）cleanup，断开对象之间的连接、访问外部资源（保存数据）
 * 6）析构，销毁所有对象
 */
class App {
  public:
    virtual ~App() {}

    /**
     * 填写默认的配置参数
     *
     * 在 initialize() 之前，main 框架会调该函数向每个 App 索要一份 App 默认的配置。
     * 在解析参数的时候，则根据参数项对该配置进行修改。再在 initialize() 的时候将修改后的配置参数传
     * 进来，作为初始化的依据。
     */
    virtual void fillDefaultConfig(Json &cfg) const { }

    /**
     * 构造
     *
     * 不建议将App里的对象构造放在App的构造函数中，建议在这里实现
     * 因为在App构造的时候，没有日志打印的支持。
     *
     * 在构造的时候，仅做内存申请这类无风相关的操作
     *
     * \param   ctx     应用运行上下文，包含一次可用资源，如：loop, thread_pool, timer, coroutine 等
     *
     * \return true     初始化成本
     * \return false    初始化失败
     */
    virtual bool construct(Context &ctx) = 0;

    /**
     * 初始化
     *
     * 做一些外部资源读取相关的操作（打开文件，解析配置）这类有失败风险的操作，以及对象之间的关联操作
     *
     * \param   cfg     配置参数 Json 数据
     *
     * \return true     初始化成本
     * \return false    初始化失败
     */
    virtual bool initialize(const Json &cfg) = 0;

    /**
     * 启动
     */
    virtual bool start() = 0;

    /**
     * 停止，是 start() 的逆操作
     */
    virtual void stop() = 0;

    /**
     * 清理，是 initialize() 的逆操作
     */
    virtual void cleanup() = 0;
};

}

#endif //TBOX_MAIN_APP_H_20211222
