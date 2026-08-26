# CLAUDE.md - flow 模块

## 模块定位

`flow` 提供业务流编排能力：层级有限状态机（HFSM）、可组合的动作（Action）、动作执行器（ActionExecutor）、事件发布/订阅，以及 Graphviz 可视化输出。

## 依赖关系

- 上游依赖：`eventx`、`event`、`util`、`base`
- 被依赖：无（独立可选模块）

## 关键组件

| 文件 | 说明 |
|------|------|
| `state_machine.h` | `StateMachine` 多层级有限状态机（`newState`/`addRoute`/`addEvent`，enter/exit 动作、guard/event 函数） |
| `action.h` | `Action` 动作基类（状态 `State`、结果 `Result`、原因 `Reason`，可配置 `Variables`） |
| `action_executor.h` | `ActionExecutor` 动作执行器（`append()` 按优先级排队执行） |
| `event.h` | `Event` 事件结构（`id` + `extra` 指针） |
| `event_publisher.h` / `event_subscriber.h` | 事件发布/订阅接口 |
| `event_publisher_impl.h` | 事件发布实现 |
| `action_reason.h` | 动作失败/阻塞原因码常量（`ACTION_REASON_*`，1000 以内为框架保留） |
| `to_graphviz.h` | `ToGraphviz()` / `ActionJsonToGraphviz()` / `StateMachineJsonToGraphviz()` 生成 Graphviz 文本 |

### 内置动作 `actions/`

- 流程控制：`sequence_action`、`parallel_action`、`if_else_action`、`if_then_action`、`loop_action`、`loop_if_action`、`repeat_action`、`switch_action`、`random_select_action`
- 基础：`function_action`、`sleep_action`、`dummy_action`、`succ_fail_action`
- 组合/包装：`assemble_action`、`composite_action`、`wrapper_action`
- 扩展：`event_action`、`execute_cmd_action`、`execute_in_thread_action`

## 注意事项

- `Action` 通过 `toJson()`/`fromJson()` 支持配置化，`to_graphviz` 可将 Action/StateMachine 导出为 Graphviz 图。
- `ActionExecutor` 暂不支持 `Action::block()` 功能（源码有 FIXME 注释）。

## 测试

- 测试文件：`state_machine_test.cpp`、`action_test.cpp`、`event_publisher_impl_test.cpp`、`action_executor_test.cpp`、`to_graphviz_test.cpp`，以及 `actions/` 下各动作的 `*_test.cpp`
- 运行：`.build/flow/test`
