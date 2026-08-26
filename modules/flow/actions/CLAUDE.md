# CLAUDE.md - flow/actions 目录

本目录是 `flow` 模块内置的动作（Action）实现集合，用于业务流编排。所有动作均继承自 `tbox::flow::Action`（`../action.h`）。

## 先导知识：Action 基类

理解各动作前，先掌握 `Action` 的核心概念：

- **状态 `State`**：`kIdle → kRunning / kPause → kFinished / kStoped`
- **结果 `Result`**：`kUnsure / kSuccess / kFail`
- **结束原因 `Reason`**：`{ code, message }`，用于传递失败/阻塞信息
- **执行轨迹 `Trace`**：`std::vector<Who>`，记录「谁」结束的
- **主动结束/阻塞**：子类通过 `finish(is_succ, reason, trace)` 结束，通过 `block(reason, trace)` 挂起等待唤醒
- **回调**：`setFinishCallback()` 结束回调、`setBlockCallback()` 阻塞回调
- **超时**：`setTimeout()` 给动作加超时，超时触发 `onTimeout()`
- **变量**：`vars()` 返回 `util::Variables`，供动作间共享数据
- **序列化**：`toJson()` 导出配置（便于可视化与调试）
- **`isReady()`**：纯虚函数，返回动作是否已配置完整可执行

> 各动作构造时**必须传入 `event::Loop &loop`**（引用，非指针）。所有动作的 `start/pause/resume/stop/reset` 都必须在 Loop 线程调用。

---

## 一、基础原子动作

这些动作是「叶子」节点，不包含子动作，通常作为组合动作的末端。

### 1. FunctionAction — 函数动作

封装一个同步执行的函数/回调，是最常用的叶子动作。

```c++
auto action = new FunctionAction(loop,
    [] { return do_something(); });  // 返回 bool：true 成功，false 失败
```

支持 4 种函数签名（按需选择）：
- `Func = std::function<bool()>`
- `FuncWithReason = std::function<bool(Reason &)>` — 可填充失败原因
- `FuncWithVars = std::function<bool(util::Variables &)>` — 可读写共享变量
- `FuncWithReasonVars = std::function<bool(Reason &, util::Variables &)>`

**运用场景**：所有需要「同步执行一段业务逻辑」的叶子动作，如：启动一个设备、写入一条配置、校验一个参数。

### 2. SleepAction — 延时动作

挂起指定时长后自动结束（成功）。

```c++
auto action = new SleepAction(loop, std::chrono::milliseconds(500));
```

支持两种时长来源：
- 固定时长 `std::chrono::milliseconds`
- 动态时长 `Generator = std::function<std::chrono::milliseconds()>`（每次启动时生成）

**运用场景**：流程中需要「等待一段时间」的步骤，如：重试前等待、设备预热延时、轮询间隔。

### 3. SuccAction / FailAction — 恒成功/恒失败

`SuccAction` 启动即成功结束；`FailAction` 启动即失败结束（用于占位/测试）。

```c++
new SuccAction(loop);  // 立即 finish(true)
new FailAction(loop);  // 立即 finish(false)
```

**运用场景**：组合动作中的占位分支（如 `IfThenAction` 的「不做事」分支）、单元测试。

### 4. DummyAction — 木偶动作

自身不会主动结束，由外部通过 `emitFinish()` / `emitBlock()` 控制其结束/阻塞，可监听 start/stop/pause/resume/reset 生命周期。

```c++
auto dummy = new DummyAction(loop);
dummy->setStartCallback([] { LogInfo("started"); });
// 外部在某个时机：
dummy->emitFinish(true);   // 让它成功结束
```

**运用场景**：作为「可控占位符」挂在组合动作中，由外部事件决定其结束时机；调试与单元测试。

### 5. ExecuteCmdAction — 执行系统命令

在 `ThreadExecutor`（线程池）中执行 shell 命令，执行完在主 Loop 线程回调结束。结果通过 `getReturnCode()` / `getStdOutput()` 获取。

```c++
auto action = new ExecuteCmdAction(loop, thread_pool, "ls -la /tmp");
```

**运用场景**：业务流程中需要调用外部命令（如生成文件、调用脚本、重启服务）的场景，避免阻塞 Loop 线程。

### 6. ExecuteInThreadAction — 线程中执行函数

在 `ThreadExecutor` 中执行一个**可能阻塞**的函数，完成后再回主线程结束。函数返回 `bool` 表示成败，可填 `Reason`。

```c++
auto action = new ExecuteInThreadAction(loop, thread_pool,
    [](Reason &reason) { return heavy_compute(reason); });
```

**运用场景**：耗时/阻塞计算（如大数据处理、密集 I/O），与 `FunctionAction` 的区别在于它运行在后台线程。

### 7. EventAction — 事件等待动作

订阅一个 `EventPublisher`，在事件到来前保持阻塞，事件处理后结束。需传入 `type` 与 `EventPublisher &`。

```c++
auto action = new EventAction(loop, "MyEventAction", event_publisher);
```

`EventAction` 本身不定义结束条件，通常需**继承并重写 `onEvent()`**（来自 `EventSubscriber`）在收到期望事件时调用 `finish()`。

**运用场景**：流程中需要「等待某个外部事件/信号」的步骤，如：等待设备上线、等待用户确认。

---

## 二、组装基类

### AssembleAction / SerialAssembleAction — 组装动作基类

- `AssembleAction`：管理子动作集合的基类，提供 `addChild()` / `addChildAs(child, role)` / `setChild()` / `setChildAs(child, role)`，以及 `setFinalCallback()`（结束/被停止时触发）。
- `SerialAssembleAction`：继承自 `AssembleAction`，提供**串行**执行子动作的通用逻辑（`startThisAction` / `stopCurrAction` / `handleChildFinishEvent`）。

> 一般业务代码不直接实例化这两个类，而是使用下面具体的组合动作，或在自定义组合动作时继承它们。

### CompositeAction — 封装组合动作

用于**把一组动作封装成一个新的、可复用的动作类**。继承它并在构造函数里组装子动作即可，无需重写其它虚函数。

```c++
class MyCompositeAction : public CompositeAction {
  public:
    MyCompositeAction(event::Loop &loop)
      : CompositeAction(loop, "MyCompositeAction")
    {
        auto seq = new SequenceAction(loop);
        seq->addChild(new FunctionAction(loop, [] { return step_a(); }));
        seq->addChild(new FunctionAction(loop, [] { return step_b(); }));
        setChild(seq);   // 设置唯一子动作
    }
};
```

**运用场景**：把反复出现的动作组合打包成语义化的新动作（如「打开设备并自检」），提升可读性与复用性。

---

## 三、流程编排动作（组合动作）

### 8. SequenceAction — 顺序动作

按顺序依次执行子动作，前一个结束后才执行下一个。

```c++
auto seq = new SequenceAction(loop, SequenceAction::Mode::kAllFinish);
seq->addChild(a1);
seq->addChild(a2);
seq->addChild(a3);
```

**模式 `Mode`**：
- `kAllFinish`：所有子动作依次执行完（默认）
- `kAnyFail`：任一子动作失败即结束（返回失败）
- `kAnySucc`：任一子动作成功即结束（返回成功）

**运用场景**：串行的业务流程，如「初始化 → 连接 → 发送数据 → 断开」。

### 9. ParallelAction — 并行动作

同时启动所有子动作，按模式决定整体结束时机。

```c++
auto par = new ParallelAction(loop, ParallelAction::Mode::kAllFinish);
par->addChild(a1);
par->addChild(a2);
```

**模式 `Mode`**：
- `kAllFinish`：所有子动作都结束才结束
- `kAnyFail`：任一失败即整体失败（其余被停止）
- `kAnySucc`：任一成功即整体成功（其余被停止）

**运用场景**：无先后依赖、可并发的多个子任务，如：同时向多个设备下发指令、并发拉取多路数据。

### 10. RepeatAction — 重复 N 次

将子动作重复执行指定次数。

```c++
auto rep = new RepeatAction(loop, child, 5, RepeatAction::Mode::kNoBreak);
```

**模式 `Mode`**：
- `kNoBreak`：`for (i=0; i<times; ++i) { action(); }`
- `kBreakFail`：`for (i=0; i<times && action(); ++i)` — 失败即停止
- `kBreakSucc`：`for (i=0; i<times && !action(); ++i)` — 成功即停止

**运用场景**：固定次数的重试/轮询，如「最多重试 3 次」。

### 11. LoopAction — 循环动作

无限/条件循环执行子动作。

```c++
auto loop = new LoopAction(loop, LoopAction::Mode::kForever);
loop->setChild(body);
```

**模式 `Mode`**：
- `kForever`：`while(true) { action(); }`
- `kUntilFail`：`while(action());`
- `kUntilSucc`：`while(!action());`

**运用场景**：守护型循环（如主循环、心跳循环、长连接收发循环）。`kForever` 需外部 `stop()` 才会结束。

### 12. LoopIfAction — 先判断后循环

`do { exec(); } while (if());` 风格：先执行 `exec`，再判断 `if` 是否继续。

```c++
auto la = new LoopIfAction(loop);
la->setChildAs(if_action, "if");    // 循环条件
la->setChildAs(exec_action, "exec"); // 循环体
la->setFinishResult(false);          // 可设定循环结束时的整体结果
```

**运用场景**：至少执行一次、之后按条件决定是否继续的循环（如「执行请求 → 判断是否还有下一页 → 继续」）。

### 13. IfElseAction — 双分支

`if (cond) then() else else()`。

```c++
auto ie = new IfElseAction(loop);
ie->setChildAs(cond, "if");
ie->setChildAs(then_action, "succ");  // 也可用 "then"
ie->setChildAs(else_action, "fail");  // 也可用 "else"
```

条件动作成功（`true`）走 then 分支，失败（`false`）走 else 分支。

**运用场景**：简单的二选一分支，如「设备在线则执行 A，否则执行 B」。

### 14. IfThenAction — 多分支（if / else-if 链）

支持多个 `if → then` 对，依次判断，命中则执行对应 then，全部未命中则整体失败。

```c++
auto it = new IfThenAction(loop);
it->addChildAs(cond1, "if");  it->addChildAs(do1, "then");
it->addChildAs(cond2, "if");  it->addChildAs(do2, "then");
it->addChildAs(condN, "if");  it->addChildAs(doN, "then");
```

**运用场景**：多级条件判断，如「按优先级选择可用的处理策略」。

### 15. SwitchAction — 多路分支

`switch (key) { case "x": ...; default: ...; }`，条件动作返回的字符串作为分支键。

```c++
auto sw = new SwitchAction(loop);
sw->setChildAs(key_action, "switch");      // 产出分支键的动作
sw->setChildAs(case_a, "case:aaa");        // case 分支，role 格式 "case:xxx"
sw->setChildAs(default_action, "default"); // 默认分支
```

**运用场景**：按枚举/状态值分发到不同处理流程，如「根据消息类型路由到不同处理器」。

### 16. RandomSelectAction — 随机选择

从子动作中随机挑选一个执行（`rand() % n`）。

```c++
auto rs = new RandomSelectAction(loop);
rs->addChild(a1);
rs->addChild(a2);
rs->addChild(a3);
```

**运用场景**：负载均衡、A/B 测试、随机策略选择。

### 17. WrapperAction — 结果包装

对唯一子动作的执行结果做变换。

```c++
auto w = new WrapperAction(loop, child, WrapperAction::Mode::kInvert);
```

**模式 `Mode`**：
- `kNormal`：透传结果
- `kInvert`：取反（成功变失败、失败变成功）
- `kAlwaySucc`：无论子动作结果如何，整体成功
- `kAlwayFail`：无论子动作结果如何，整体失败

**运用场景**：调整某个动作的语义，如「将『设备不存在』视为流程成功」「忽略某步的失败」。

---

## 四、JSON 配置格式（`toJson`）

组合动作支持导出 JSON（用于调试、可视化，见 `../to_graphviz.h`）。典型格式示例：

```jsonc
// SequenceAction
{ "type": "Sequence", "mode": "AllFinish", "index": 0,
  "children": [ { /* child1 */ }, { /* child2 */ } ] }

// LoopAction
{ "type": "Loop", "mode": "Forever", "child": { /* body */ } }

// RepeatAction
{ "type": "Repeat", "repeat_times": 5, "remain_times": 2, "child": { /* body */ } }

// IfElseAction
{ "type": "IfElse", "children": {
    "0.if":  { /* cond */ },
    "1.then": { /* then */ },
    "2.else": { /* else */ } } }

// SwitchAction
{ "type": "Switch", "children": {
    "00.switch": { /* key */ },
    "01.caseA":  { /* case A */ },
    "99.default":{ /* default */ } } }
```

> 可通过 `flow::ToGraphviz()` 将动作树转换为 Graphviz 文本，结合 `tools/graphviz_render` 实时查看流程图。

---

## 五、各动作测试文件对照

| 动作 | 测试文件 |
|------|---------|
| AssembleAction | `assemble_action_test.cpp` |
| CompositeAction | `composite_action_test.cpp` |
| FunctionAction | `function_action_test.cpp` |
| SleepAction | `sleep_action_test.cpp` |
| Succ/FailAction | `succ_fail_action_test.cpp` |
| ExecuteCmdAction | `execute_cmd_action_test.cpp` |
| ExecuteInThreadAction | `execute_in_thread_action_test.cpp` |
| SequenceAction | `sequence_action_test.cpp` |
| ParallelAction | `parallel_action_test.cpp` |
| RepeatAction | `repeat_action_test.cpp` |
| LoopAction | `loop_action_test.cpp` |
| LoopIfAction | `loop_if_action_test.cpp` |
| IfElseAction | `if_else_action_test.cpp` |
| IfThenAction | `if_then_action_test.cpp` |
| SwitchAction | `switch_action_test.cpp` |
| RandomSelectAction | `random_select_action_test.cpp` |
| WrapperAction | `wrapper_action_test.cpp` |

运行测试：`.build/flow/test`
