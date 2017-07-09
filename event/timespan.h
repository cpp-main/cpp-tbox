#ifndef TBOX_EVENT_TIMESPAN_H_20170709
#define TBOX_EVENT_TIMESPAN_H_20170709

#include <time.h>

namespace tbox {
namespace event {

class Timespan {
  public:
    Timespan() {
        val_.tv_sec  = 0;
        val_.tv_usec = 0;
    }

    Timespan(int sec, int msec) {
        val_.tv_sec  = sec;
        val_.tv_usec = msec * 1000;
    }

    operator struct timeval () const { return val_; }

    static Timespan Zero() { return Timespan(); }
    static Timespan Millisecond(int msec) { return Timespan(msec/1000, msec%1000); }
    static Timespan Second(int sec) { return Timespan(sec, 0); }
    static Timespan Minute(int m) { return Timespan(m * 60, 0); }
    static Timespan Hour(int h) { return Timespan(h * 3600, 0); }
    static Timespan Day(int d) { return Timespan(d * 86400, 0); }

  private:
    struct timeval val_;
};

}
}

#endif //TBOX_EVENT_TIMESPAN_H_20170709
