# Alarm Module (alarm)

## What is it?

The alarm module provides multiple alarm types: CronAlarm (Linux cron expressions), OneshotAlarm (one-shot), WeeklyAlarm (weekly recurring), and WorkdayAlarm (workdays/holidays). They are implemented based on the TimerEvent of the event module and support independent timezone settings.

## Why do you need it?

In service-oriented programs, scheduled tasks are one of the most common requirements. The alarm module provides multiple scheduling strategies to meet different scenarios: execute at a fixed time every day, execute on specific days of the week, flexibly schedule via cron expressions, execute only on workdays, etc.

## Header Files

```cpp
#include <tbox/alarm/alarm.h>              //! Alarm base class
#include <tbox/alarm/cron_alarm.h>         //! Cron expression alarm
#include <tbox/alarm/oneshot_alarm.h>      //! One-shot alarm
#include <tbox/alarm/weekly_alarm.h>       //! Weekly alarm
#include <tbox/alarm/workday_alarm.h>       //! Workday alarm
#include <tbox/alarm/workday_calendar.h>    //! Workday calendar
```

## Core Classes and Interfaces

### Alarm — Alarm Base Class

All alarm types share the following interfaces:

| Method | Description |
|------|------|
| `Alarm(loop)` | Constructor, specify the event loop |
| `setCallback(cb)` | Set the timer trigger callback |
| `setTimezone(offset_minutes)` | Set timezone offset (east zones are positive, e.g. UTC+8 = 480) |
| `enable()` | Enable the timer |
| `disable()` | Disable the timer |
| `isEnabled()` | Check if the timer is enabled |
| `refresh()` | Refresh (should be called after clock synchronization) |
| `remainSeconds()` | Get remaining seconds |
| `cleanup()` | Clean up resources |

### Alarm Types Comparison

| Type | Initialization Parameters | Applicable Scenarios |
|------|------|------|
| **CronAlarm** | cron expression string | Flexible scheduling, e.g. "every minute", "the 1st of each month" |
| **OneshotAlarm** | seconds_of_day | Execute once at a fixed time today or tomorrow |
| **WeeklyAlarm** | seconds_of_day + week_mask | Execute on specific days of the week |
| **WorkdayAlarm** | seconds_of_day + calendar + workday | Execute only on workdays or holidays |

### CronAlarm — Cron Expression Alarm

Cron expression format (6 fields):

```
second  minute  hour  day  month  weekday
*       *       *     *    *      *
```

Examples:
- `"18 28 14 * * *"` — Every day at 14:28:18
- `"0 30 8 * * 1-5"` — Monday through Friday at 8:30:00
- `"0 0 12 1 * *"` — The 1st of each month at 12:00:00

### OneshotAlarm — One-shot Alarm

The `seconds_of_day` parameter is the number of seconds from local 00:00 to the trigger time.

```
08:30 = 8 × 3600 + 30 × 60 = 30600
```

If the specified time has already passed (current time is later than the scheduled time), it will execute tomorrow.

### WeeklyAlarm — Weekly Alarm

`week_mask` is a fixed-length 7-character string starting from Sunday. `'1'` means execute, other characters mean skip:

```
"0111110" → Monday through Friday
"1111111" → Every day
"1000001" → Only Sunday and Saturday
```

### WorkdayCalendar — Workday Calendar

WorkdayCalendar provides date query functionality to WorkdayAlarm:

| Method | Description |
|------|------|
| `updateSpecialDays(days)` | Update special holiday/makeup workday schedule |
| `updateWeekMask(mask)` | Modify the default weekly workday mask (default: Monday through Friday) |
| `subscribe(alarm)` | Subscribe an alarm (automatically notified when calendar changes) |
| `unsubscribe(alarm)` | Unsubscribe an alarm |
| `isWorkay(day_index)` | Query whether the specified date is a workday |

### WorkdayAlarm — Workday Alarm

The `workday` parameter: true = execute only on workdays, false = execute only on holidays.

## Usage Examples

### CronAlarm — Execute at 14:28:18 every day

> Full example at `examples/alarm/cron_alarm/`

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
    tmr.initialize("18 28 14 * * *");  //! Every day at 14:28:18
    tmr.setCallback([] { LogInfo("time is up"); });
    tmr.enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    LogOutput_Disable();
    return 0;
}
```

### OneshotAlarm — Execute at 8:30 every morning

> Full example at `examples/alarm/oneshot_alarm/`

```cpp
alarm::OneshotAlarm tmr(sp_loop);
tmr.initialize(30600);  //! 08:30 = 30600 seconds
tmr.setCallback([] { LogInfo("time is up"); });
tmr.enable();
```

### WeeklyAlarm — Execute at 8:30 Monday through Friday

> Full example at `examples/alarm/weekly_alarm/`

```cpp
alarm::WeeklyAlarm tmr(sp_loop);
tmr.initialize(30600, "0111110");  //! Monday through Friday at 08:30
tmr.setCallback([] { LogInfo("time is up"); });
tmr.enable();
```

### Setting an Independent Timezone

```cpp
tmr.setTimezone(480);  //! UTC+8 (+8 × 60 = 480 minutes)
//! If not set, the system timezone is used by default
```

### WorkdayAlarm — Execute Only on Workdays

```cpp
alarm::WorkdayCalendar calendar;
//! Mark January 1, 2024 as a holiday
calendar.updateSpecialDays({{19723, false}});  //! day_index = days from 1970-1-1

alarm::WorkdayAlarm tmr(sp_loop);
tmr.initialize(30600, &calendar, true);  //! true = only workdays
tmr.setCallback([] { LogInfo("workday alarm"); });
tmr.enable();

//! When the calendar is updated, subscribed alarms will automatically refresh
calendar.updateSpecialDays({{19724, true}});  //! Makeup workday
```

### refresh() — Refresh After Clock Synchronization

```cpp
//! Refresh the timer after system clock synchronization to ensure accurate trigger times
tmr.refresh();
```

## Common Scenarios

1. **Daily execution at a fixed time**: OneshotAlarm with seconds_of_day
2. **Weekly execution on specific days**: WeeklyAlarm with week_mask
3. **Flexible cron scheduling**: CronAlarm with cron expressions
4. **Execute only on workdays**: WorkdayAlarm + WorkdayCalendar
5. **Cross-timezone scheduling**: setTimezone() to set an independent timezone

## Important Notes

1. **seconds_of_day calculation**: Counted from local 00:00, e.g. 08:30 = 30600
2. **OneshotAlarm "tomorrow" behavior**: If the current time has already passed the scheduled time, it will execute tomorrow
3. **WorkdayCalendar lifetime**: The calendar object passed to WorkdayAlarm must live longer than the WorkdayAlarm
4. **week_mask format**: Fixed 7 characters, starting from Sunday, '1' marks execution days
5. **When to call refresh()**: If the system clock is inaccurate when enable() is called, the scheduled task will also be inaccurate; refresh should be called after clock synchronization

## Related Modules

- **event**: Alarm is implemented based on TimerEvent for timing
- **base**: Provides logging, ScopeExit, and other infrastructure
