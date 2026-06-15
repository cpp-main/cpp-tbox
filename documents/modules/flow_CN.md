# 流程控制模块 (flow)

## 是什么？

flow 模块提供了两类流程控制工具：多层级有限状态机（StateMachine）和行为树（Action 系列）。StateMachine 用于状态驱动的业务逻辑，Action 用于组合型复杂流程。

## 为什么需要它？

在事件驱动编程中，复杂业务流程需要管理大量状态和条件判断。StateMachine 提供清晰的状态定义和转换规则，Action 提供组合型流程控制（顺序、并发、条件选择等），将复杂的控制逻辑从业务代码中分离出来。

![state-machine](../images/0010-state-machine-graph.png)

![action-tree](../images/0010-action-tree-graph.jpg)

## 头文件

```cpp
#include <tbox/flow/state_machine.h>      //! 状态机
#include <tbox/flow/action.h>              //! 动作基类
#include <tbox/flow/action_executor.h>     //! 动作执行器
#include <tbox/flow/event.h>               //! 事件定义
#include <tbox/flow/event_publisher.h>      //! 事件发布器
#include <tbox/flow/event_subscriber.h>     //! 事件订阅器
#include <tbox/flow/to_graphviz.h>         //! 导出 Graphviz 图
```

## 核心类与接口

### StateMachine — 多层级有限状态机

| 方法 | 说明 |
|------|------|
| `newState(state_id, enter_action, exit_action, label)` | 创建状态 |
| `addRoute(from, event, to, guard, action, label)` | 添加状态转换路由 |
| `addEvent(state_id, event_id, action)` | 添加状态内事件处理 |
| `setInitState(state_id)` | 设置起始状态 |
| `setSubStateMachine(state_id, sub_sm)` | 设置子状态机（层级嵌套） |
| `setStateChangedCallback(cb)` | 设置状态变更回调 |
| `start()` | 启动状态机 |
| `stop()` | 停止状态机 |
| `run(event)` | 运行状态机（传入事件） |
| `currentState()` | 获取当前状态 |
| `lastState()` | 获取上一个状态 |
| `nextState()` | 获取下一个状态（转换中有效） |
| `isRunning()` | 是否运行中 |
| `isTerminated()` | 是否已终止 |

#### 状态机关键概念

- **StateID**：状态编号，0 为终止状态，-1 为无效状态
- **EventID**：事件编号，0 表示任意事件
- **Route**：定义从某状态收到某事件后转换到另一状态的条件路由
- **GuardFunc**：条件判定函数，返回 true 表示条件成立可转换
- **EventFunc**：事件处理函数，返回 >=0 表示需要转换到指定状态
- **子状态机**：StateMachine 支持嵌套，内部状态机可以独立管理子状态

### Action — 行为树动作基类

Action 是行为树的基础节点，提供统一的生命周期管理：

| 方法 | 说明 |
|------|------|
| `start()` | 开始执行 |
| `pause()` | 暂停 |
| `resume()` | 恢复 |
| `stop()` | 停止 |
| `reset()` | 重置到初始状态 |
| `isReady()` | 是否准备就绪（需子类实现） |
| `setFinishCallback(cb)` | 设置完成回调 |
| `setBlockCallback(cb)` | 设置阻塞回调 |
| `setTimeout(ms)` | 设置超时时间 |
| `finish(is_succ, why, trace)` | 主动结束 |
| `block(why, trace)` | 主动暂停 |

Action 状态：kIdle（空闲）→ kRunning（运行）→ kFinished（完成）/kStoped（停止）/kPause（暂停）

Action 结果：kUnsure（未知）→ kSuccess（成功）/kFail（失败）

#### Action 子类类型

Action 有多种组合和功能子类（完整示例见单元测试用例 `modules/flow/`）：

- **顺序组合**：SequentialAction（按顺序执行多个子动作）
- **并发组合**：ParallelAction（同时执行多个子动作）
- **条件选择**：IfElseAction（条件分支）、SwitchAction（多路选择）
- **循环控制**：LoopAction（循环执行）、WhileAction（条件循环）
- **装饰器**：RetryAction（失败重试）、TimeoutAction（超时控制）、DelayAction（延迟启动）
- **事务**：TransactionAction（全部成功则提交，任一失败则回滚）

### Event — 事件定义

```cpp
struct Event {
    using ID = int;
    ID id = 0;
    const void *extra = nullptr;  //! 附带数据指针
};
```

支持从枚举类型创建：`Event(MyEvent::kTimeout, &data)`

## 使用示例

### 状态机 — 简单开关

> 完整示例见单元测试用例 `modules/flow/`

```cpp
enum State { kOff = 1, kOn = 2 };
enum Event { kToggle = 10 };

StateMachine sm;

//! 创建两个状态
sm.newState(kOff, [](Event) { LogInfo("enter OFF"); }, [](Event) { LogInfo("exit OFF"); });
sm.newState(kOn, [](Event) { LogInfo("enter ON"); }, [](Event) { LogInfo("exit ON"); });

//! 添加转换路由：任何状态收到 Toggle 事件都切换到另一个状态
sm.addRoute(kOff, kToggle, kOn, nullptr, nullptr);
sm.addRoute(kOn, kToggle, kOff, nullptr, nullptr);

sm.start();            //! 从第一个状态 kOff 开始
sm.run(Event(kToggle)); //! OFF → ON
sm.run(Event(kToggle)); //! ON → OFF
```

### 状态机 — 条件路由

```cpp
//! 带条件判断的路由：仅当 guard 返回 true 时才转换
sm.addRoute(kIdle, kRequest, kBusy,
    [](Event ev) { return /* 某条件 */; },
    nullptr
);
```

### 子状态机嵌套

```cpp
StateMachine outer_sm;
StateMachine inner_sm;

//! 内部状态机定义
inner_sm.newState(kInnerA, ...);
inner_sm.newState(kInnerB, ...);

//! 将内部状态机挂到外部状态机的某个状态上
outer_sm.newState(kOuterState, ...);
outer_sm.setSubStateMachine(kOuterState, &inner_sm);
```

### 行为树 — 顺序执行

> 完整示例见单元测试用例 `modules/flow/`

```cpp
//! SequentialAction：按顺序执行多个子动作
//! 子动作A完成后自动启动B，B完成后启动C
//! 任一失败则整体失败
```

### 导出 Graphviz 图

```cpp
Json js;
sm.toJson(js);  //! 导出状态机为 JSON，可用于可视化
//! 使用 to_graphviz.h 可转换为 Graphviz 格式
```

## 常见场景

1. **设备状态管理**：如 IoT 设备的空闲→运行→故障→恢复状态流转
2. **协议状态机**：如 TCP 连接状态、HTTP 请求处理状态
3. **业务流程编排**：如订单创建→支付→发货→完成，失败则回滚
4. **条件分支**：根据传感器数据选择不同的执行路径
5. **重试机制**：使用 RetryAction 在失败时自动重试

## 注意事项

1. **StateID 0 是终止状态**：状态机到达 0 号状态自动终止，不需要额外处理
2. **子状态机生命期**：setSubStateMachine() 传入的子状态机生命期需比父状态机长
3. **Action 的 finish/block**：finish() 表示正常结束（成功或失败），block() 表示暂停等待外部条件
4. **Event.extra 指针**：extra 指针指向的数据生命期需在事件处理期间有效
5. **toJson 导出**：状态机可导出为 JSON 用于调试和可视化

## 相关模块

- **event**：基于 Loop 运行状态机和动作
- **util**：Action 使用 Variables 存储变量
- **base**：提供 Json、Log 等基础设施
