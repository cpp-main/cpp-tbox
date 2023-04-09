#include "stat.h"
#include <iomanip>

using namespace std;

ostream& operator<< (ostream &os, const tbox::event::Stat &stat)
{
    os << "stat_time: " << stat.stat_time_us << " us" << endl;
    os << "time_cost: " << stat.time_cost_us << " us" << endl;
    os << "peak_time_cost: " << stat.peak_cost_us << " us" << endl;
    os << "event_count: " << stat.event_count << endl;

    if (stat.event_count != 0)
        os << "avg_cost: " << stat.time_cost_us / stat.event_count << " us" << endl;

    if (stat.stat_time_us != 0)
        os << "event_cpu: " << stat.time_cost_us * 100.0 / stat.stat_time_us << " %" << endl;

    os << "wait_cost: " << stat.wait_cost_us << " us" << endl;
    os << "wait_count: " << stat.wait_count << endl;

    double wake_cpu = ((stat.stat_time_us - stat.wait_cost_us) * 100.0 / stat.stat_time_us);
    os << "wake_cpu: " << wake_cpu << " %" << endl;

    os << "run_in_loop_peak_num: " << stat.run_in_loop_peak_num << endl;
    os << "run_next_peak_num: " << stat.run_next_peak_num << endl;

    return os;
}
