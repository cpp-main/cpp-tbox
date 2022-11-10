#include <sys/syscall.h>
#include <unistd.h>

#include <thread>
#include <mutex>
#include <chrono>
#include <map>
#include <memory>

#include "thread_wdog.h"
#include <tbox/base/log.h>

namespace tbox {
namespace util {

namespace {

void OnLoopDie(pid_t tid, const std::string &name);

class LoopInfo {
  public:
    void initialize(const std::string &name, uint16_t sec)
    {
        thread_name_ = name;
        timeout_sec_ = sec;
        remain_sec_ = sec;
    }

    void resetRemainSec()
    {
        remain_sec_ = timeout_sec_;
    }

    bool decRemainSecAndCheck()
    {
        if (remain_sec_ > 0) {
            --remain_sec_;
            return false;
        } else {
            return true;
        }
    }

    std::string getName() const { return thread_name_; }

  private:
    std::string thread_name_;    //! 线程名
    uint16_t timeout_sec_ = 0;   //! 线程超时时间长度
    uint16_t remain_sec_ = 0;    //! 剩余时间长度
};

using PidToLoopInfo = std::map<pid_t, std::shared_ptr<LoopInfo> >;

PidToLoopInfo pid_to_thread_info;         //! pid --> 线程信息表
std::mutex      pid_to_thread_info_mutex;   //! 锁
std::shared_ptr<std::thread> sp_thread;     //! 线程对象
bool keep_running = false;                  //! 线程是否继续工作标记
LoopWDog::LoopDieCallback thread_die_cb = OnLoopDie;  //! 回调函数

pid_t CurrentLoopID()
{
    return ::syscall(SYS_gettid);
}

void UpdateAllLoopInfo()
{
    std::lock_guard<std::mutex> g(pid_to_thread_info_mutex);
    for (auto &item : pid_to_thread_info) {
        auto tid = item.first;
        auto sp_info = item.second;
        if (sp_info->decRemainSecAndCheck()) {
            if (thread_die_cb)
                thread_die_cb(tid, sp_info->getName());
        }
    }
}

//! 监控线程函数
void LoopProc()
{
    while (keep_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        UpdateAllLoopInfo();
    }
}

//! 默认线程超时执行函数
void OnLoopDie(const std::string &name)
{
    LogWarn("loop \"%s\" die!", name.c_str());
}

}

void LoopWDog::SetLoopDieCallback(const LoopDieCallback &cb)
{
    thread_die_cb = cb;
}

void LoopWDog::Start()
{
    keep_running = true;
    sp_thread = std::make_shared<std::thread>(LoopProc);
}

void LoopWDog::Register(const std::string &thread_name, uint16_t timeout_sec)
{
    pid_t curr_pid = CurrentLoopID();

    std::shared_ptr<LoopInfo> sp_info;

    std::lock_guard<std::mutex> g(pid_to_thread_info_mutex);
    auto iter = pid_to_thread_info.find(curr_pid);
    if (iter == pid_to_thread_info.end()) { //! 如果没有找到那么创建
        LogInfo("new thread \"%s\"", thread_name.c_str());
        auto sp_new_info = std::make_shared<LoopInfo>();
        pid_to_thread_info.insert(std::make_pair(curr_pid, sp_new_info));
        sp_info = sp_new_info;
    } else {
        LogInfo("update thread");
        sp_info = iter->second;
    }

    sp_info->initialize(thread_name, timeout_sec);
}

void LoopWDog::Unregister()
{
    pid_t curr_pid = CurrentLoopID();

    LogInfo("delete thread");

    std::lock_guard<std::mutex> g(pid_to_thread_info_mutex);
    pid_to_thread_info.erase(curr_pid);
}

void LoopWDog::FeedDog()
{
    pid_t curr_pid = CurrentLoopID();

    std::lock_guard<std::mutex> g(pid_to_thread_info_mutex);
    auto iter = pid_to_thread_info.find(curr_pid);
    if (iter != pid_to_thread_info.end()) { //! 如果没有找到那么创建
        auto sp_info = iter->second;
        sp_info->resetRemainSec();
    } else {
        LogWarn("no current thread");
    }
}

void LoopWDog::Stop()
{
    keep_running = false;
    sp_thread->join();

    pid_to_thread_info.clear();
    sp_thread.reset();
}

}
}
