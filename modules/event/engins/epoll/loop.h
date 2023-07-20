#ifndef TBOX_EVENT_EPOLL_LOOP_H_20220105
#define TBOX_EVENT_EPOLL_LOOP_H_20220105

#include <unordered_map>

#include "../../common_loop.h"

#include <tbox/base/object_pool.hpp>
#include "types.h"

#ifndef DEFAULT_MAX_LOOP_ENTRIES
#define DEFAULT_MAX_LOOP_ENTRIES (256)
#endif

namespace tbox {
namespace event {

class EpollLoop : public CommonLoop {
  public:
    explicit EpollLoop();
    virtual ~EpollLoop() override;

  public:
    virtual void runLoop(Mode mode) override;

    virtual FdEvent* newFdEvent(const std::string &what) override;

  public:
    inline int epollFd() const { return epoll_fd_; }

    EpollFdSharedData* refFdSharedData(int fd);
    void unrefFdSharedData(int fd);

  protected:
    virtual void stopLoop() override { keep_running_ = false; }

  private:
    int  max_loop_entries_ = DEFAULT_MAX_LOOP_ENTRIES;
    int  epoll_fd_ = -1;
    bool keep_running_ = true;

    std::unordered_map<int, EpollFdSharedData*> fd_data_map_;
    ObjectPool<EpollFdSharedData> fd_shared_data_pool_{64};
};

}
}

#endif //TBOX_EVENT_EPOLL_LOOP_H_20220105
