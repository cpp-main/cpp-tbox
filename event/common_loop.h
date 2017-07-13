#ifndef TBOX_EVENT_COMMON_LOOP_H_20170713
#define TBOX_EVENT_COMMON_LOOP_H_20170713

#include <list>
#include <mutex>
#include <thread>

#include "loop.h"

namespace tbox {
namespace event {

class CommonLoop : public Loop {
  public:
    CommonLoop();
    virtual ~CommonLoop();

  public:
    virtual bool isInLoopThread();
    virtual void runInLoop(const RunInLoopFunc &func);

  protected:
    void runThisBeforeLoop();
    void runThisAfterLoop();

    void onGotRunInLoopFunc(short);

    void commitRequest();
    void finishRequest();

  private:
    std::mutex lock_;

    std::thread::id loop_thread_id_;

    bool has_unhandle_req_;
    int read_fd_, write_fd_;
    FdItem *sp_read_item_;
    std::list<RunInLoopFunc> func_list_;
};

}
}

#endif //TBOX_EVENT_COMMON_LOOP_H_20170713
