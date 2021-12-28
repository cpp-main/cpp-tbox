#ifndef TBOX_MAIN_MAIN_H_20211225
#define TBOX_MAIN_MAIN_H_20211225

#include "app.h"
#include "apps.h"
#include "context.h"

namespace tbox::main {
/**
 * 应用注册函数
 *
 * 当程序运行起来时，用调用该函数，加载 App 应用对象。
 * 用户需要去定义它。
 * 例：
 * namespace tbox::main {
 *     void RegisterApps(Context &context, Apps &apps)
 *     {
 *         apps.add(new MyApp1(context));
 *         apps.add(new MyApp2(context));
 *     }
 * }
 */
void RegisterApps(Context &context, Apps &apps);

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
}

#endif //TBOX_MAIN_MAIN_H_20211225
