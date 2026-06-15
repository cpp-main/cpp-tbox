# 定时闹钟模块 (alarm)

## 是什么？

alarm 模块提供了多种定时闹钟类型：CronAlarm（Linux cron 表达式）、OneshotAlarm（一次性）、WeeklyAlarm（每周循环）、WorkdayAlarm（工作日/节假日）。它们基于 event 模块的 TimerEvent 实现，支持独立时区设置。

## 为什么需要它？

在服务型程序中，定时任务是最常见的需求之一。alarm 模块提供了多种定时策略，满足不同场景：每天固定时间执行、每周特定日期执行、按 cron 表达式灵活调度、仅在工作日执行等。

## 头文件

```cpp
#include <tbox/alarm/alarm.h>              //! 闹钟基类
#include <tbox/alarm/cron_alarm.h>         //! Cron 表达式闹钟
#include <tbox/alarm/oneshot_alarm.h>      //! 一次性闹钟
#include <tbox/alarm/weekly_alarm.h>       //! 每周闹钟
#include <tbox/alarm/workday_alarm.h>       //! 工作日闹钟
#include <tbox/alarm/workday_calendar.h>    //! 工作日日历
```

## 核心类与接口

### Alarm — 闹钟基类

所有闹钟类型共享以下接口：

| 方法 | 说明 |
|------|------|
| `Alarm(loop)` | 构造，指定事件循环 |
| `setCallback(cb)` | 设置定时触发回调 |
| `setTimezone(offset_minutes)` | 设置时区偏移（东区为正，如东8区=480） |
| `enable()` | 使能定时器 |
| `disable()` | 关闭定时器 |
| `isEnabled()` | 定时器是否已使能 |
| `refresh()` | 刷新（时钟同步后应调用） |
| `remainSeconds()` | 获取剩余秒数 |
| `cleanup()` | 清理资源 |

### 闹钟类型对比

| 类型 | 初始化参数 | 适用场景 |
|------|------|------|
| **CronAlarm** | cron 表达式字符串 | 灵活调度，如"每分钟"、"每月1号" |
| **OneshotAlarm** | seconds_of_day | 每天或明天固定时间执行一次 |
| **WeeklyAlarm** | seconds_of_day + week_mask | 每周特定日期执行 |
| **WorkdayAlarm** | seconds_of_day + calendar + workday | 仅工作日或节假日执行 |

### CronAlarm — Cron 表达式闹钟

Cron 表达式格式（6个字段）：

```
秒  分  时  日  月  星期
*   *   *   *   *   *
```

示例：
- `"18 28 14 * * *"` — 每天 14:28:18
- `"0 30 8 * * 1-5"` — 周一到周五 8:30:00
- `"0 0 12 1 * *"` — 每月1号 12:00:00

### OneshotAlarm — 一次性闹钟

`seconds_of_day` 参数是从本地 00:00 起到触发时间的秒数。

```
08:30 = 8 × 3600 + 30 × 60 = 30600
```

如果指定时间已过（当前时间晚于定时时间），将在明天执行。

### WeeklyAlarm — 每周闹钟

`week_mask` 为固定长度 7 个字符的字符串，星期日开始，`'1'` 表示执行，其他表示不执行：

```
"0111110" → 周一到周五执行
"1111111" → 每天执行
"1000001" → 仅周日和周六执行
```

### WorkdayCalendar — 工作日日历

WorkdayCalendar 用于向 WorkdayAlarm 提供日期查询功能：

| 方法 | 说明 |
|------|------|
| `updateSpecialDays(days)` | 更新特殊节假日/补班日期表 |
| `updateWeekMask(mask)` | 修改一周默认工作日（默认周一到周五） |
| `subscribe(alarm)` | 订阅闹钟（日历变更时自动通知） |
| `unsubscribe(alarm)` | 取消订阅 |
| `isWorkay(day_index)` | 查询指定日期是否为工作日 |

### WorkdayAlarm — 工作日闹钟

`workday` 参数：true=仅工作日执行，false=仅节假日执行。

## 使用示例

### CronAlarm — 每天 14:28:18 执行

> 完整示例见 `examples/alarm/cron_alarm/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/alarm/cron_alarm.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    alarm::CronAlarm tmr(sp_loop);
    tmr.initialize("18 28 14 * * *");  //! 每天 14:28:18
    tmr.setCallback([] { LogInfo("time is up"); });
    tmr.enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    LogOutput_Disable();
    return 0;
}
```

### OneshotAlarm — 每天早上 8:30 执行

> 完整示例见 `examples/alarm/oneshot_alarm/`

```cpp
alarm::OneshotAlarm tmr(sp_loop);
tmr.initialize(30600);  //! 08:30 = 30600秒
tmr.setCallback([] { LogInfo("time is up"); });
tmr.enable();
```

### WeeklyAlarm — 周一到周五 8:30 执行

> 完整示例见 `examples/alarm/weekly_alarm/`

```cpp
alarm::WeeklyAlarm tmr(sp_loop);
tmr.initialize(30600, "0111110");  //! 周一到周五 08:30
tmr.setCallback([] { LogInfo("time is up"); });
tmr.enable();
```

### 设置独立时区

```cpp
tmr.setTimezone(480);  //! 东8区（+8 × 60 = 480分钟）
//! 不设置时默认使用系统时区
```

### WorkdayAlarm — 仅工作日执行

```cpp
alarm::WorkdayCalendar calendar;
//! 标注2024年1月1日为节假日
calendar.updateSpecialDays({{19723, false}});  //! day_index从1970-1-1起的天数

alarm::WorkdayAlarm tmr(sp_loop);
tmr.initialize(30600, &calendar, true);  //! true=仅工作日
tmr.setCallback([] { LogInfo("workday alarm"); });
tmr.enable();

//! 日历更新后，订阅了该日历的闹钟会自动刷新
calendar.updateSpecialDays({{19724, true}});  //! 补班日
```

### refresh() — 时钟同步后刷新

```cpp
//! 系统时钟同步后刷新定时器，确保触发时间准确
tmr.refresh();
```

## 常见场景

1. **每天定时执行**：OneshotAlarm 指定 seconds_of_day
2. **每周特定日期执行**：WeeklyAlarm 指定 week_mask
3. **灵活 cron 调度**：CronAlarm 使用 cron 表达式
4. **仅工作日执行**：WorkdayAlarm + WorkdayCalendar
5. **跨时区定时**：setTimezone() 设置独立时区

## 注意事项

1. **seconds_of_day 计算**：从本地 00:00 起算，如 08:30 = 30600
2. **OneshotAlarm 的"明天"行为**：如果当前时间已过定时时间，将在明天执行
3. **WorkdayCalendar 生命期**：传入 WorkdayAlarm 的 calendar 对象生命期必须比 WorkdayAlarm 长
4. **week_mask 格式**：固定 7 个字符，星期日开始，'1' 为执行标记
5. **refresh() 的时机**：系统时钟不准时 enable() 的定时任务也不准，时钟同步后应刷新

## 相关模块

- **event**：Alarm 基于 TimerEvent 实现定时
- **base**：提供日志、ScopeExit 等基础设施
