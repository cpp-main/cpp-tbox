# 应用框架模块 (main)

## 是什么？

main 模块是应用程序的启动框架，对程序启动过程进行了统一完备的封装，让开发者只需关心业务逻辑，不必关心启动流程。它自动创建事件循环、线程池、定时池、协程调度器等公共组件，并通过 Context 对象提供给业务模块使用。

## 为什么需要它？

开发服务型程序时，通常需要重复编写以下流程：创建事件循环、初始化日志、配置线程池、处理命令行参数、响应退出信号等。main 模块将这些流程统一封装，开发者只需实现业务模块的初始化、启动、停止、清理四个步骤即可。

![main-framework](../images/0008-main-framework.png)

## 头文件

```cpp
#include <tbox/main/main.h>       //! 主入口函数与注册接口
#include <tbox/main/module.h>     //! 模块基类
#include <tbox/main/context.h>    //! 进程上下文
#include <tbox/main/args_parser.h> //! 命令行参数解析器
#include <tbox/main/log.h>        //! 日志相关
#include <tbox/main/trace.h>      //! 追踪相关
```

## 核心类与接口

### Main / Start / Stop — 启动与停止

| 函数 | 说明 |
|------|------|
| `Main(argc, argv)` | 在前端运行 tbox::main 框架，阻塞直到收到停止信号 |
| `Start(argc, argv)` | 在后端运行 tbox::main 框架，不阻塞 |
| `Stop()` | 停止后端运行的 tbox::main 框架 |
| `RaiseStopSignal()` | 给自身发送停止请求 |

### Module — 业务模块基类

Module 的生命周期遵循以下过程：

```
构造 → 初始化 → 启动 → .运行中. → 停止 → 清理 → 析构
```

以使用一台电脑为类比：
1. **构造** — 将设备逐一布置好
2. **初始化 (initialize)** — 插好电源，连接线缆
3. **启动 (start)** — 启动各个设备
4. ... 正常工作 ...
5. **停止 (stop)** — 关闭各个设备
6. **清理 (cleanup)** — 断开连接线缆
7. **析构** — 将设备逐一撤走

| 方法 | 说明 |
|------|------|
| `Module(name, ctx)` | 构造函数，name 为模块名，ctx 为进程上下文 |
| `add(child, required)` | 添加子模块。required=true 时子模块初始化/启动失败会导致整个程序启动失败 |
| `addAs(child, name, required)` | 添加子模块并重新命名 |
| `name()` | 获取模块名 |
| `ctx()` | 获取进程上下文 |
| `state()` | 获取模块状态（kNone/kInited/kRunning） |

需要重写的虚函数：

| 虚函数 | 说明 |
|------|------|
| `onFillDefaultConfig(Json)` | 填充默认配置参数（注意：此阶段日志系统不可用） |
| `onInit(const Json &cfg)` | 初始化，读取配置、建立对象连接 |
| `onStart()` | 启动模块，令对象开始工作 |
| `onStop()` | 停止模块，对应 onStart() 的逆操作 |
| `onCleanup()` | 清理模块，对应 onInit() 的逆操作 |

### Context — 进程上下文

Context 提供了框架创建的公共组件，业务模块通过 `ctx()` 获取：

| 接口 | 说明 |
|------|------|
| `ctx.loop()` | 事件循环对象 |
| `ctx.thread_pool()` | 线程池对象 |
| `ctx.timer_pool()` | 定时池对象 |
| `ctx.async()` | 异步操作对象 |
| `ctx.terminal()` | 交互终端对象 |
| `ctx.coroutine()` | 协程调度器对象 |
| `ctx.running_time()` | 程序运行时长 |
| `ctx.start_time_point()` | 程序启动时间点 |
| `ctx.args()` | 命令行参数列表 |

### 必须实现的函数

开发者需要实现以下函数供框架调用：

| 函数 | 说明 |
|------|------|
| `RegisterApps(Module &apps, Context &ctx)` | 注册应用模块 |
| `GetAppDescribe()` | 返回应用描述（执行 -h 时显示） |
| `GetAppBuildTime()` | 返回编译时间（执行 -v 时显示），通常返回 `__DATE__ " " __TIME__` |
| `GetAppVersion(major, minor, rev, build)` | 设置应用版本号 |

## 使用示例

### 单一应用

> 完整示例见 `examples/main/01_one_app/`

**第一步**：继承 Module 类

```cpp
// app.h
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

```cpp
// app.cpp
#include "app.h"
#include <tbox/base/log.h>

App::App(tbox::main::Context &ctx) : Module("app", ctx)
{
    LogTag();
}

bool App::onInit(const tbox::Json &cfg) { LogTag(); return true; }
bool App::onStart() { LogTag(); return true; }
void App::onStop() { LogTag(); }
void App::onCleanup() { LogTag(); }
```

**第二步**：实现注册函数

```cpp
// main.cpp
#include <tbox/main/main.h>
#include "app.h"

namespace tbox {
namespace main {

void RegisterApps(Module &apps, Context &ctx) {
    apps.add(new ::App(ctx));
}

std::string GetAppDescribe() { return "One app sample"; }
std::string GetAppBuildTime() { return __DATE__ " " __TIME__; }

void GetAppVersion(int &major, int &minor, int &rev, int &build) {
    major = 0; minor = 0; rev = 1; build = 0;
}

}}
```

**第三步**：在 Makefile 中添加依赖库

```makefile
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

### 多个应用模块

> 完整示例见 `examples/main/02_more_than_one_apps/`

```cpp
void RegisterApps(Module &apps, Context &ctx) {
    apps.add(new App1(ctx));       //! 必须启动模块
    apps.add(new App2(ctx), false); //! 非必须启动模块，失败不影响其他模块
}
```

### 子模块嵌套

Module 支持树状嵌套结构，父模块自动管理子模块的生命周期：

```cpp
class ParentApp : public tbox::main::Module {
  public:
    ParentApp(Context &ctx) : Module("parent", ctx) {
        add(new SubModuleA(ctx));      //! 作为子模块添加
        add(new SubModuleB(ctx));
    }
};

//! 子模块的 initialize/start/stop/cleanup 由父模块自动调用
//! 不可私自 delete 或手动调用子模块的生命周期方法
```

### 后端运行模式

> 完整示例见 `examples/main/06_run_in_backend/`

当需要将 tbox::main 框架集成到已有程序框架时，可使用后端运行模式：

```cpp
int main(int argc, char **argv) {
    if (!tbox::main::Start(argc, argv))
        return 0;

    //! 原有程序框架继续运行
    while (true) {
        // ...
    }

    tbox::main::Stop();
    return 0;
}
```

## 常见场景

1. **标准服务程序**：使用 `Main()` 在前端运行，业务模块通过 Context 获取公共组件
2. **嵌入式集成**：使用 `Start()/Stop()` 将框架集成到已有程序，不影响原有架构
3. **模块化开发**：不同功能模块独立继承 Module，通过 `RegisterApps` 组合
4. **可选模块**：使用 `add(child, false)` 添加非必需模块，失败不影响主流程

## 注意事项

1. **Module 生命期顺序**：必须按 构造→initialize→start→stop→cleanup→析构 顺序，不可跳跃
2. **子模块不要手动管理**：add() 后子模块生命期由父模块管控，不可私自 delete 或调用生命周期方法
3. **onFillDefaultConfig 中日志不可用**：该阶段日志系统尚未初始化，不要使用 LogInfo 等宏
4. **required 参数的影响**：required=true 的子模块 onInit/onStart 失败会导致整个程序启动失败
5. **RegisterApps 函数签名**：必须放在 `tbox::main` namespace 中，否则框架找不到

## 相关模块

- **event**：框架自动创建 Loop，通过 `ctx.loop()` 获取
- **eventx**：框架自动创建 ThreadPool/TimerPool/Async，通过 `ctx.thread_pool()/ctx.timer_pool()/ctx.async()` 获取
- **terminal**：框架自动创建 Terminal，通过 `ctx.terminal()` 获取
- **coroutine**：框架自动创建 Scheduler，通过 `ctx.coroutine()` 获取
- **base**：提供日志、ScopeExit 等基础组件
- **log**：框架自动配置日志系统
