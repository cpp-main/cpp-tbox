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
 * main框架主入口
 *
 * 正常情况下，用户无需调用它。main架构默认会调用。
 * 如果用户还想在程序启动前做些其它的动作，可以重新定义 main()，然后再调用它。
 * 例：
 * int main(int argc, char **argv)
 * {
 *     DoSomethingBeforeStart();
 *     return tbox::main::Main(argc, argv);
 * }
 */
int Main(int argc, char **argv);

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

}
}

#endif //TBOX_MAIN_MAIN_H_20211225
