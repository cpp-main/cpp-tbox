# 介绍

main，是应用程序的启动框架。 
它对程序启动过程进行了统一完备的封装，让开发者只需关心业务逻辑，不必关心启动流程。

# 使用方法

开发者只需要三步走：

**第一步**：继承 tbox::main::Module 类，如 App 类；

```C++
#include <tbox/main/main.h>

class App : public tbox::main::Module
{
  public:
    App(tbox::main::Context &ctx);
    ~App();

  protected:
    virtual bool onInit(const tbox::Json &cfg) override;
    virtual bool onStart() override;
    virtual void onStop() override;
    virtual void onCleanup() override;
};

```
重写 onInit(), onStart(), onStop(), onCleanup() 虚函数，实现自定义模块在不同时期下的动作。
如果没有动作，则不用重写对应的虚函数。

**第二步**：实现应用基础的几个函数`RegisterApps()`,`GetAppDescribe()`,`GetAppBuildTime()`,`GetAppVersion()`

```C++
#include <tbox/main/main.h>
#include "app.h"

namespace tbox {
namespace main {

void RegisterApps(Module &apps, Context &ctx)
{
    apps.add(new ::App(ctx));
}

std::string GetAppDescribe()
{
    return "One app sample";
}

std::string GetAppBuildTime()
{
    return __DATE__ " " __TIME__;
}

void GetAppVersion(int &major, int &minor, int &rev, int &build)
{
    major = 0;
    minor = 0;
    rev = 1;
    build = 0;
}

}
}
```

**第三步**：在 Makefile 中添加 main 所依赖的库：

```Makefile
LDFLAGS += -L.. \
	-ltbox_main \
	-ltbox_terminal \
	-ltbox_network \
	-ltbox_eventx \
	-ltbox_event \
	-ltbox_util \
	-ltbox_base \
	-lpthread -ldl
```
