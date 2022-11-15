#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <memory>
#include <algorithm>

#include "loop_wdog.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>

namespace tbox {
namespace eventx {

namespace {

void OnLoopBlock(const std::string &name);

struct LoopInfo {
  LoopInfo(event::Loop *l, const std::string &n) :
    loop(l), name(n),
    tag(std::make_shared<bool>(true))
  { }

  event::Loop*  loop;
  std::string   name;
  std::shared_ptr<bool> tag;
};

using LoopInfoVec = std::vector<LoopInfo>;

LoopInfoVec     _loop_info_vec; //! 线程信息表
std::mutex      _mutex_lock;    //! 锁
std::thread*    _sp_thread = nullptr; //! 线程对象
bool _keep_running = false;     //! 线程是否继续工作标记

LoopWDog::LoopBlockCallback _loop_die_cb = OnLoopBlock;  //! 回调函数

void SendLoopFunc() {
  std::lock_guard<std::mutex> lg(_mutex_lock);
  for (auto &loop_info : _loop_info_vec) {
    if (loop_info.loop->isRunning()) {
      auto tag = loop_info.tag;
      if (*tag) {
        *tag = false;
        loop_info.loop->runInLoop([tag] { *tag = true; });
      }
    }
  }
}

void CheckLoopTag() {
  std::lock_guard<std::mutex> lg(_mutex_lock);
  for (auto loop_info: _loop_info_vec) {
    auto tag = loop_info.tag;
    if (!(*tag)) {
      _loop_die_cb(loop_info.name);
    }
  }
}

//! 监控线程函数
void ThreadProc() {
  while (_keep_running) {
    SendLoopFunc();

    for (int i = 0; i < 10 && _keep_running; ++i)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (_keep_running)
      CheckLoopTag();

    for (int i = 0; i < 40 && _keep_running; ++i)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

//! 默认线程超时执行函数
void OnLoopBlock(const std::string &name) {
  LogWarn("loop \"%s\" block!", name.c_str());
}

}

void LoopWDog::SetLoopBlockCallback(const LoopBlockCallback &cb) {
  assert(cb != nullptr);
  _loop_die_cb = cb;
}

void LoopWDog::Start() {
  std::lock_guard<std::mutex> lg(_mutex_lock);
  if (!_keep_running) {
    _keep_running = true;
    _sp_thread = new std::thread(ThreadProc);
  }
}

void LoopWDog::Stop() {
  std::lock_guard<std::mutex> lg(_mutex_lock);
  if (_keep_running) {
    _keep_running = false;
    _sp_thread->join();
    CHECK_DELETE_RESET_OBJ(_sp_thread);
    _loop_info_vec.clear();
  }
}

void LoopWDog::Register(event::Loop *loop, const std::string &name) {
  std::lock_guard<std::mutex> lg(_mutex_lock);
  auto iter = std::find_if(_loop_info_vec.begin(), _loop_info_vec.end(),
    [loop, name] (const LoopInfo &loop_info) {
      return loop_info.loop == loop;
    }
  );

  if (iter == _loop_info_vec.end()) { //! 如果没有找到那么创建
    _loop_info_vec.emplace_back(LoopInfo(loop, name));
  }
}

void LoopWDog::Unregister(event::Loop *loop) {
  std::lock_guard<std::mutex> lg(_mutex_lock);
  auto iter = std::remove_if(_loop_info_vec.begin(), _loop_info_vec.end(),
    [loop] (const LoopInfo &loop_info) {
      return loop_info.loop == loop;
    }
  );

  if (iter != _loop_info_vec.end()) {
    _loop_info_vec.erase(iter, _loop_info_vec.end()); 
  }
}

}
}
