/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_MAIN_MODULE_H_20220326
#define TBOX_MAIN_MODULE_H_20220326

#include <vector>

#include <tbox/base/json_fwd.h>
#include <tbox/util/variables.h>

#include "context.h"

namespace tbox {
namespace main {

/**
 * 模块类
 *
 * 模块的生命期遵循以下几个过程：
 *
 *   构造 --> 初始化 --> 启动 --.
 *                              | 运行中
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
 * 对应模块各步骤的动作：
 * 1）构造，构建应用相关的对象
 * 2）initialize，访问外部资源（读取文件）、建立对象之间的连接
 * 3）start，启动所有对象，令它们开始工作
 * 4）stop， 停止所有对象，令它们停止工作
 * 5）cleanup，断开对象之间的连接、访问外部资源（保存数据）
 * 6）析构，销毁所有对象
 *
 * 在Module中，已定义了initialize(),start(),stop(),cleanup()函数，无需重写
 * 使用者仅需要根据本模块的自身需要，重写onInit(),onStart(),onStop(),onCleanup()函数
 * 以实现对应步骤所执行的操作
 */
class Module {
  public:
    /**
     * \brief   构造
     *
     * \param   name        模块名，可以为""
     * \param   ctx         进程上下文，含loop, thread_pool, time_pool, terminal 等公用组件
     *
     * \warn    在构造函数中日志打印不会生效，因为构造时日志系统还没有初始化
     */
    explicit Module(const std::string &name, Context &ctx);
    virtual ~Module();

    //! 状态
    enum class State {
        kNone,      //!< 仅完成了构造，还没有initialize()
        kInited,    //!< 已initialize()，还没有start()
        kRunning    //!< 已start()
    };

  public:
    /**
     * \brief   添加子模块
     *
     * \param   child       子模块对象地址
     * \param   required    是否为必须启动模块
     *                      如果为true，那么这个模块的onInit(),onStart()都必须成功，否则整个程序启动失败
     *                      如果为false，该模块失败，不会影响程序其它模块
     *
     * \return  true    成功
     * \return  false   失败，只有在同一个对象地址重复add()时出现
     *
     * \warn    一旦将子Module添加到父Module，子Module的生命期就由父Module管控
     *          不可私自delete，也不要再手动调子Module的initialize(),start(),stop(),cleanup()等函数
     *          应交由父Module自行管理
     */
    bool add(Module *child, bool required = true);

    //! 同上，但会对child进行重新命名
    bool addAs(Module *child, const std::string &name, bool required = true);

    //! 下面5个函数，由父Module自动调用。使用者不需要关心
    void fillDefaultConfig(Json &js_parent);
    bool initialize(const Json &js_parent);
    bool start();
    void stop();
    void cleanup();

    inline std::string name() const { return name_; }
    inline Context& ctx() const { return ctx_; }
    inline State state() const { return state_; }
    inline util::Variables& vars() { return vars_; }

    //! 导出为JSON对象
    virtual void toJson(Json &js) const;

  protected:
    //! 下面的5个虚函数，可由使用者根据需要重写。如果没有操作，就不用重写

    //! 填充本模块的默认参数，可重写。注意：日志系统在该函数执行过程中尚不可用
    virtual void onFillDefaultConfig(Json &js_this) { (void)js_this; }
    //! 初始化本模块的操作，可重写
    virtual bool onInit(const Json &js_this) { (void)js_this; return true; }
    //! 启动本模块的操作，可重写
    virtual bool onStart() { return true; }
    //! 停止本模块的操作，可重写，对应onStart()的逆操作
    virtual void onStop() { }
    //! 清理本模块的操作，可重写，对应onInit()的逆操作
    virtual void onCleanup() { }

  private:
    std::string name_;
    Context &ctx_;

    struct ModuleItem {
        Module *module_ptr;
        bool    required;   //! 是否为必须
    };
    std::vector<ModuleItem> children_;
    State state_ = State::kNone;

    Module *parent_ = nullptr;
    util::Variables vars_;
};

}
}

#endif //TBOX_MAIN_MODULE_H_20220326
