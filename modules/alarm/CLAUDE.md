# CLAUDE.md - alarm 模块

## 模块定位

`alarm` 提供基于事件循环的各种定时闹钟：单次、每周、工作日/节假日、Cron 表达式。

## 依赖关系

- 上游依赖：`event`、`base`，以及第三方 `3rd-party/ccronexpr`
- 被依赖：无（独立可选模块）

## 关键组件

| 文件 | 说明 |
|------|------|
| `alarm.h` | `Alarm` 定时器基类：`setCallback()`、`setTimezone()`、`enable()`/`disable()`、`refresh()`、`remainSeconds()`；子类实现 `calculateNextLocalTimeSec()` |
| `oneshot_alarm.h` | `OneshotAlarm` 单次定时（指定当日秒数，已过期则次日触发） |
| `weekly_alarm.h` | `WeeklyAlarm` 每周定时（`week_mask` 7 位掩码，周日开始） |
| `workday_alarm.h` | `WorkdayAlarm` 工作日/节假日定时（依赖 `WorkdayCalendar`） |
| `workday_calendar.h` | `WorkdayCalendar` 工作日日历（判断某日是否工作日） |
| `cron_alarm.h` | `CronAlarm` Linux cron 表达式定时（秒分时日月星期，6 段） |
| `3rd-party/ccronexpr.cpp` | 第三方 cron 解析库 |

## 关键概念

- `kSecondsOfDay` / `kSecondsOfWeek` 常量。
- 时区：`setTimezone(offset_minutes)`，东区为正；不设置则随系统时区。
- `refresh()`：系统时钟校准后调用，重新计算定时点。

## 注意事项

- `WorkdayAlarm` 要求 `wp_calendar` 指向的 `WorkdayCalendar` 生命期比闹钟对象长。
- `CronAlarm` 表达式格式为 6 段（含秒），不同于标准 5 段 crontab。

## 测试

- 测试文件：`workday_calendar_test.cpp`（`workday_calendar.cpp` 一并编译）
- 运行：`.build/alarm/test`

## 示例

- `examples/alarm/`：cron_alarm、oneshot_alarm、weekly_alarm
