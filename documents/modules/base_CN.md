# 基础组件模块 (base)

## 是什么？

base 模块是 cpp-tbox 最底层的依赖模块，提供了日志宏、断言、对象池、生命期标签、作用域退出、储物柜、通用定义等基础设施。所有其他模块都依赖 base 模块。

## 为什么需要它？

在 C++ 项目开发中，日志打印、资源管理、断言检查是最基础的需求。base 模块将这些常用功能统一封装，使得所有模块共享一致的日志输出接口和资源管理模式，避免重复实现。

## 头文件

```cpp
#include <tbox/base/log.h>              //! 日志宏
#include <tbox/base/log_impl.h>         //! 日志实现细节
#include <tbox/base/log_output.h>       //! 日志输出开关
#include <tbox/base/assert.h>           //! 断言宏
#include <tbox/base/defines.h>          //! 通用宏定义（NONCOPYABLE 等）
#include <tbox/base/scope_exit.hpp>     //! 作用域退出动作
#include <tbox/base/object_pool.hpp>    //! 对象池
#include <tbox/base/lifetime_tag.hpp>   //! 生命期标签
#include <tbox/base/cabinet.hpp>        //! 储物柜
#include <tbox/base/cabinet_token.h>    //! 储物柜凭据
#include <tbox/base/catch_throw.h>      //! 异常捕获
#include <tbox/base/memblock.h>         //! 内存块
#include <tbox/base/recorder.h>         //! 记录器
#include <tbox/base/backtrace.h>        //! 调用栈回溯
#include <tbox/base/json.hpp>           //! JSON 库（nlohmann/json）
#include <tbox/base/json_fwd.h>         //! JSON 前置声明
#include <tbox/base/version.h>          //! 版本信息
```

## 核心组件

### 日志宏 (log.h)

日志是项目中最常用的功能。base 模块定义了 8 个日志级别和对应的打印宏：

| 日志级别 | 宏 | 说明 |
|---------|------|------|
| FATAL (0) | `LogFatal(fmt, ...)` | 程序将崩溃 |
| ERROR (1) | `LogErr(fmt, ...)` | 严重问题，程序无法处理 |
| WARN (2) | `LogWarn(fmt, ...)` | 内部异常，但程序可处理 |
| NOTICE (3) | `LogNotice(fmt, ...)` | 不大严重但应关注，如无效输入 |
| IMPORTANT (4) | `LogImportant(fmt, ...)` | 重要消息 |
| INFO (5) | `LogInfo(fmt, ...)` | 正常消息 |
| DEBUG (6) | `LogDbg(fmt, ...)` | 程序内部调试信息 |
| TRACE (7) | `LogTrace(fmt, ...)` | 临时调试日志 |

辅助宏：

| 宏 | 说明 |
|------|------|
| `LogTag()` | 打印 "==> Run Here <=="，标记代码执行位置 |
| `LogUndo()` | 打印 "!!! Undo !!!"，标记未实现功能 |
| `LogErrno(err, fmt, ...)` | 打印 errno 错误码及其含义 |

> **注意**：`LogDbg` 和 `LogTrace` 可通过 `STATIC_LOG_LEVEL` 编译选项在编译时屏蔽，减少生产环境的日志量。

#### MODULE_ID 定义

日志输出时会附带模块标识。需要在编译选项中定义 `MODULE_ID`，如 `-DMODULE_ID=alarm`。若未定义，日志中模块名显示为 "???"。

#### 日志输出开关 (log_output.h)

```cpp
LogOutput_Enable();   //! 开启日志输出到 stdout
LogOutput_Disable();  //! 关闭日志输出
```

> `LogOutput_Enable()` 是最简单的日志输出方式，将日志打印到 stdout。更丰富的日志配置请使用 **log 模块**。

### 断言宏 (assert.h)

```cpp
TBOX_ASSERT(expr);  //! 调试模式下，条件不成立时打印 LogFatal 并 abort()
```

- 在 `NDEBUG` 模式下（Release 编译），`TBOX_ASSERT` 不执行任何操作
- 在调试模式下，断言失败会打印错误信息并终止程序

### 作用域退出动作 (scope_exit.hpp)

`SetScopeExitAction` 宏用于在代码块退出时自动执行指定动作，类似于 Go 的 defer。

```cpp
SetScopeExitAction(action);  //! 在当前作用域退出时执行 action
```

**典型用法**：管理动态分配资源的释放。

```cpp
Loop* sp_loop = Loop::New();
SetScopeExitAction([sp_loop] { delete sp_loop; });
//! ... 使用 sp_loop ...
//! 函数退出时自动 delete sp_loop
```

> **注意**：`SetScopeExitAction` 创建的 `ScopeExitActionGuard` 对象不可复制和移动（NONCOPYABLE/IMMOVABLE）。可通过 `cancel()` 取消执行。

### 对象池 (object_pool.hpp)

`ObjectPool<T>` 是一个模板类，用于减少频繁 new/delete 对象的性能开销。通过空闲块链表缓存已释放的内存块，避免反复分配与释放内存。

```cpp
//! 创建对象池
ObjectPool<MyStruct> op;
//! 或指定保留空闲块数量
ObjectPool<MyStruct> op(64);

//! 分配对象（等价于 new，但更快）
auto p1 = op.alloc(1, "hello");  //! 支持构造参数

//! 释放对象（等价于 delete）
op.free(p1);

//! 获取统计数据
auto stat = op.getStat();
//! stat.total_alloc_times  — 总分配次数
//! stat.total_free_times   — 总释放次数
//! stat.peak_alloc_number  — 最大同时分配数
//! stat.peak_free_number   — 最大空闲缓存数
```

> **重要**：凡是使用 ObjectPool 分配的对象，**一定要使用 ObjectPool 进行释放**，不可用 `delete`。

### 生命期标签 (lifetime_tag.hpp)

`LifetimeTag` 用于标记对象的生命期是否有效，解决"指针指向的对象被提前析构"的风险问题。

```cpp
struct HostObject {
    int value = 0;
    LifetimeTag tag;    //! 生命期标签
};

HostObject *o = new HostObject;
LifetimeTag::Watcher w = o->tag;  //! 创建观察器

if (w)    //! true，对象存活
    cout << "value:" << o->value << endl;

delete o;  //! 析构对象

if (w)    //! false，对象已析构
    cout << "不能安全访问" << endl;
```

> **注意**：目前 LifetimeTag 未做加锁保护，不支持多线程。

### 储物柜 (cabinet.hpp)

`Cabinet<T>` 是一个带凭据（Token）的安全对象容器。通过 Token 存取对象，即使对象被删除，旧的 Token 也不会误取到新对象。

```cpp
Cabinet<MyClass> cab;

//! 存入对象，获取 Token
auto token = cab.alloc(new MyClass);

//! 取出对象
auto p = cab.free(token);  //! 取出并删除记录，返回对象指针

//! 检查 Token 是否有效
auto p2 = cab.at(token);   //! 不取出，仅查看
```

### 通用定义 (defines.h)

| 宏 | 说明 |
|------|------|
| `NONCOPYABLE(classname)` | 禁止拷贝构造和赋值操作 |
| `IMMOVABLE(classname)` | 禁止移动构造和赋值操作 |
| `DECLARE_COPY_FUNC(classname)` | 声明拷贝函数（供 Variables 使用） |
| `CHECK_DELETE_RESET_OBJ(ptr)` | delete 指针并置 nullptr |

## 使用示例

### 打印日志

> 完整示例见 `examples/base/print_log/`

```cpp
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

#define MODULE_ID "my_app"

int main() {
    LogOutput_Enable();

    LogInfo("program started");
    LogDbg("debug info: count=%d", 42);
    LogWarn("unexpected input: %s", "abc");
    LogErr("file open failed");

    LogOutput_Disable();
    return 0;
}
```

### 断言检查

> 完整示例见 `examples/base/assert/`

```cpp
#include <tbox/base/assert.h>
#include <tbox/base/log_output.h>

int main() {
    LogOutput_Enable();

    int value = 10;
    TBOX_ASSERT(value > 0);   //! 调试模式下，条件成立则正常继续
    //! TBOX_ASSERT(value < 0); //! 条件不成立时，打印 LogFatal 并 abort

    LogOutput_Disable();
    return 0;
}
```

### 对象池

> 完整示例见 `examples/base/object_pool/`

```cpp
#include <tbox/base/object_pool.hpp>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

class MyStruct {
  public:
    MyStruct(int i, const std::string &s) : i_(i), s_(s) { }
    void print() { LogInfo("i:%d, s:%s", i_, s_.c_str()); }
  private:
    int i_;
    std::string s_;
};

int main() {
    LogOutput_Enable();

    ObjectPool<MyStruct> op;

    auto p1 = op.alloc(1, "hello");  //! 等价于 new MyStruct(1, "hello")
    p1->print();

    op.free(p1);  //! 等价于 delete p1，但内存块被缓存

    //! 再次分配时复用之前缓存的内存块，避免 malloc
    auto p2 = op.alloc(2, "world");
    p2->print();
    op.free(p2);

    auto stat = op.getStat();
    LogInfo("alloc:%zu, free:%zu, peak:%zu",
            stat.total_alloc_times, stat.total_free_times, stat.peak_alloc_number);

    LogOutput_Disable();
    return 0;
}
```

### 生命期标签

> 完整示例见 `examples/base/lifetime_tag/`

```cpp
#include <tbox/base/lifetime_tag.hpp>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

struct Resource {
    int data = 100;
    tbox::LifetimeTag tag;
};

int main() {
    LogOutput_Enable();

    Resource *res = new Resource;
    tbox::LifetimeTag::Watcher watcher = res->tag;

    LogInfo("alive: %d, data: %d", (bool)watcher, res->data);

    delete res;  //! 对象析构

    LogInfo("alive: %d", (bool)watcher);  //! watcher 为 false
    //! 不能再安全访问 res->data

    LogOutput_Disable();
    return 0;
}
```

## 常见场景

1. **日志打印**：所有模块统一使用 `LogInfo/LogErr/LogDbg` 等宏打印日志
2. **资源自动释放**：使用 `SetScopeExitAction` 在函数退出时自动 delete/new 的对象
3. **高频对象分配**：使用 `ObjectPool` 减少频繁 new/delete 的性能开销
4. **指针安全检查**：使用 `LifetimeTag` + `Watcher` 检查对象是否存活
5. **禁用拷贝/移动**：使用 `NONCOPYABLE/IMMOVABLE` 宏保护类的语义完整性

## 注意事项

1. **MODULE_ID 必须定义**：未定义时日志中模块名显示为 "???"，影响日志定位
2. **ObjectPool 的 free vs delete**：通过 ObjectPool 分配的对象只能用 ObjectPool 的 `free()` 释放，不能用 `delete`
3. **LifetimeTag 不支持多线程**：目前未加锁保护，仅适用于单线程或 Loop 线程内
4. **SetScopeExitAction 不可跨作用域**：它的执行时机取决于所在代码块的退出，注意 lambda 捕获的对象生命周期
5. **TBOX_ASSERT 在 Release 下无效**：NDEBUG 编译时断言被忽略，不要用断言替代错误处理

## 相关模块

- **log**：基于 base/log.h 的日志通道实现，提供文件/stdout/syslog 等输出方式
- **event**：依赖 base 的 Cabinet、ObjectPool、defines 等
- **所有模块**：base 是所有模块的基础依赖
