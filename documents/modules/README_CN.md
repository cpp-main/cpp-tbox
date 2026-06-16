# cpp-tbox 模块使用文档

cpp-tbox 是一个基于事件驱动的 C++ 服务应用开发库，提供完整的服务程序开发框架。

## 模块依赖关系

![modules-dependence](../images/modules-dependence.png)

## 模块列表

| 模块 | 中文名 | 功能简述 | 文档链接 |
|------|--------|---------|---------|
| **event** | 事件驱动 | 事件循环与 IO/定时/信号事件 | [event_CN.md](event_CN.md) |
| **base** | 基础组件 | 日志宏、断言、对象池、生命期标签等 | [base_CN.md](base_CN.md) |
| **main** | 应用框架 | 程序启动框架，Module 生命周期管理 | [main_CN.md](main_CN.md) |
| **eventx** | 事件扩展 | 线程池、定时池、LoopThread、异步操作 | [eventx_CN.md](eventx_CN.md) |
| **network** | 网络通信 | TCP/UDP/UART 通信与字节流抽象 | [network_CN.md](network_CN.md) |
| **terminal** | 交互终端 | 运行时命令交互，类似 Bash shell | [terminal_CN.md](terminal_CN.md) |
| **log** | 日志通道 | 文件/stdout/syslog 等日志输出 | [log_CN.md](log_CN.md) |
| **http** | HTTP 服务 | Express 式 HTTP 服务端与客户端、中间件 | [http_CN.md](http_CN.md) |
| **websocket** | WebSocket 服务 | WebSocket 服务端与客户端，RFC 6455，基于 HTTP 中间件 | [websocket_CN.md](websocket_CN.md) |
| **coroutine** | 协程 | 协程调度器与 Channel/Mutex 等辅助组件 | [coroutine_CN.md](coroutine_CN.md) |
| **alarm** | 定时闹钟 | Cron/Oneshot/Weekly/Workday 定时器 | [alarm_CN.md](alarm_CN.md) |
| **util** | 工具集 | Buffer/Json/序列化/UUID/Base64 等 17+ 工具 | [util_CN.md](util_CN.md) |
| **mqtt** | MQTT 客户端 | MQTT 协议客户端，支持 TLS 和自动重连 | [mqtt_CN.md](mqtt_CN.md) |
| **flow** | 流程控制 | 多层级状态机与行为树 | [flow_CN.md](flow_CN.md) |
| **jsonrpc** | JSON-RPC | JSON-RPC 2.0 协议实现 | [jsonrpc_CN.md](jsonrpc_CN.md) |
| **trace** | 性能追踪 | 函数级性能追踪与二进制记录 | [trace_CN.md](trace_CN.md) |
| **crypto** | 加密 | MD5 消息摘要与 AES 加密解密 | [crypto_CN.md](crypto_CN.md) |
| **dbus** | D-Bus 集成 | D-Bus 总线与事件循环集成 | [dbus_CN.md](dbus_CN.md) |
| **run** | 模块运行器 | 动态加载业务模块 .so 并运行 | [run_CN.md](run_CN.md) |

## 快速入门

### 最简单的程序

```cpp
// app.cpp
#include <tbox/main/main.h>
#include <tbox/base/log.h>

class App : public tbox::main::Module {
  public:
    App(tbox::main::Context &ctx) : Module("app", ctx) { }
    bool onStart() override { LogInfo("started"); return true; }
    void onStop() override { LogInfo("stopped"); }
};

namespace tbox { namespace main {
void RegisterApps(Module &apps, Context &ctx) { apps.add(new ::App(ctx)); }
std::string GetAppDescribe() { return "my first tbox app"; }
std::string GetAppBuildTime() { return __DATE__ " " __TIME__; }
void GetAppVersion(int &major, int &minor, int &rev, int &build) { major = 0; minor = 1; rev = 0; build = 0; }
}}
```

### 编译与运行

```bash
# 编译
g++ -o myapp app.cpp -ltbox_main -ltbox_terminal -ltbox_network \
    -ltbox_eventx -ltbox_event -ltbox_util -ltbox_base -lpthread -ldl

# 运行
./myapp          # 前端运行，按 Ctrl+C 退出
./myapp -d       # 后台运行
./myapp -h       # 显示帮助
./myapp -v       # 显示版本
```

### 核心概念

1. **事件循环 (event::Loop)**：所有异步事件的调度中心
2. **模块 (main::Module)**：业务逻辑的载体，遵循 initialize → start → stop → cleanup 生命周期
3. **回调驱动**：所有异步操作通过回调函数通知结果
4. **单线程模型**：事件循环在单线程中处理所有事件回调，跨线程操作通过 runInLoop() 注入

### 推荐学习顺序

1. [base_CN](base_CN.md) — 了解日志、ScopeExit 等基础
2. [event_CN](event_CN.md) — 理解事件循环机制
3. [main_CN](main_CN.md) — 掌握程序框架和 Module 生命周期
4. 根据业务需要选择其他模块

## 通用模式

### 初始化 → 启动 → 停止 → 清理

几乎所有 tbox 组件遵循相同的生命周期模式：

```cpp
Component comp(loop);
comp.initialize(config);   //! 初始化配置
comp.setCallback([] { ... });  //! 设置回调
comp.start();  // 或 comp.enable()  //! 启动/使能
// ... 正常运行 ...
comp.stop();   // 或 comp.disable() //! 停止/禁用
comp.cleanup();                //! 清理资源
```

### SetScopeExitAction 资源管理

```cpp
auto ptr = new SomeObject;
SetScopeExitAction([ptr] { delete ptr; });  //! 作用域退出时自动释放
```

### 跨线程任务注入

```cpp
// 其它线程向 Loop 注入任务
sp_loop->runInLoop([] { LogInfo("task in loop thread"); });

// 不确定线程时自动选择
sp_loop->run([] { LogInfo("auto route task"); });
```

## 参考图片

| 图片 | 说明 |
|------|------|
| ![tbox-loop](../images/0001-tbox-loop.jpg) | 事件循环工作原理 |
| ![main-framework](../images/0008-main-framework.png) | main 模块框架结构 |
| ![modules-dependence](../images/modules-dependence.png) | 模块依赖关系图 |
| ![state-machine](../images/0010-state-machine-graph.png) | 状态机示例 |
| ![action-tree](../images/0010-action-tree-graph.jpg) | 行为树示例 |
| ![trace-view](../images/0011-trace-view.png) | 性能追踪可视化 |
