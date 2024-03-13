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
#ifndef TBOX_MAIN_MAIN_H_20211225
#define TBOX_MAIN_MAIN_H_20211225

#include "context.h"
#include "module.h"

namespace tbox {
namespace main {

//////////////////////////////////////////////////////////////////////////
// 以下是由main库提供的
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   在前端运行tbox::main框架
 *
 * 正常情况下，用户无需调用它。tbox::main架构默认会调用。
 * 如果用户还想在程序启动前做些其它的动作，可以重新定义 main()，然后再调用它。
 *
 * 示例：
 *  int main(int argc, char **argv) {
 *    DoSomethingBeforeStart();
 *    return tbox::main::Main(argc, argv);
 *  }
 *
 * \note    该函数会阻塞，直致收到停止相关信号SIGINT,SIGTERM
 */
int Main(int argc, char **argv);

/**
 * \brief 启动tbox::main框架，并运行在后端
 *
 * 与Main()相比，Start()不会阻塞。它会在后端创建独立的线程运行tbox::main框架。
 * 它允许用户在不影响原程序框架的情况下，将tbox::main框架集成进去。
 *
 * 示例：
 *  int main(int argc, char **argv) {
 *    if (tbox::main::Start(argc, argv))
 *      return 0;
 *
 *    while (true) {
 *      // origin framework
 *    }
 *
 *    tbox::main::Stop();
 *    return 0;
 *  }
 */
bool Start(int argc, char **argv);

/**
 * \brief   停止后端运行的tbox::main框架
 *
 * 它给后端的tbox::main框架架发停止信号，并等待其正常退出。
 */
void Stop();

/**
 * 获取 main 库的版本号
 */
void GetVersion(int &major, int &minor, int &rev, int &build);

//////////////////////////////////////////////////////////////////////////
// 以下是需要开发者去实现的
//////////////////////////////////////////////////////////////////////////

/**
 * 应用注册函数，用户需要去定义它
 *
 * 当程序运行起来时，用调用该函数，加载 App 应用对象。
 *
 * 实现如下例：
 * namespace tbox {
 * namespace main {
 *     void RegisterApps(Module &apps, Context &ctx)
 *     {
 *         apps.add(new MyApp1("app1", ctx));
 *         apps.add(new MyApp2("app2", ctx));
 *     }
 * }
 * }
 */
void RegisterApps(Module &apps, Context &ctx);

/**
 * 获取应用的描述
 *
 * 在执行 -h --help 时用于显示
 */
std::string GetAppDescribe();

/**
 * 获取应用的编译时间
 *
 * 在执行 -v --version 时用于显示
 *
 * 通常这样实现：
 *   return __DATE__ " " __TIME__;
 * 但要保证每次make的时候都能重新编译
 */
std::string GetAppBuildTime();

/**
 * 获取应用的版本号，由开发者去实现
 *
 * 在执行 -v --version 时用于显示
 */
void GetAppVersion(int &major, int &minor, int &rev, int &build);

/**
 * 异常退出处理
 *
 * 当程序出现异常，即将退出前会被框架自动调用
 */
void OnAbnormalExit();

}
}

#endif //TBOX_MAIN_MAIN_H_20211225
