# 介绍

main，是应用程序的启动框架。 
它对程序启动过程进行了统一完备的封装，让开发者只需关心业务逻辑，不必关心启动流程。

# 使用方法

开发者只需要三步走：

**第一步**：实现 tbox::main::App 接口类，如 App 类；

``
#include <tbox/main/main.h>

class App : public tbox::main::App
{
  public:
    App(tbox::main::Context &ctx);
    ~App();

    bool initialize() override;
    bool start() override;
    void stop() override;
    void cleanup() override;
};

``

**第二步**：实现应用注册函数 ``tbox::main::RegisterApps()``，在该函数中创建 MyApp 类，并加入到 Apps 里；

``
#include <tbox/main/main.h>
#include "app.h"

namespace tbox::main {
void RegisterApps(Context &context, Apps &apps)
{
    apps.add(new ::App(context));
}
}
``

**第三步**：在 Makefile 中添加 main 所依赖的库：

``
LDFLAGS += \
	-ltbox_main \
	-ltbox_eventx \
	-ltbox_event-ne \
	-ltbox_util \
	-ltbox_base \
	-levent_core \
	-lev -lpthread
``
