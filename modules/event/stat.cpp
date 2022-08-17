#include "stat.h"
#include <iomanip>

using namespace std;

ostream& operator<< (ostream &os, const tbox::event::Stat &stat)
{
    os << "stat_time: " << stat.stat_time_us << " us" << endl;
    os << "time_cost: " << stat.time_cost_us << " us" << endl;
    os << "max_time_cost: " << stat.max_cost_us << " us" << endl;
    os << "event_count: " << stat.event_count << endl;

    if (stat.event_count != 0)
        os << "avg_cost: " << stat.time_cost_us / stat.event_count << " us" << endl;

    if (stat.stat_time_us != 0)
        os << "cpu: " << stat.time_cost_us * 100.0 / stat.stat_time_us << " %" << endl;

    return os;
}
