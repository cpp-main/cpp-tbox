/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "async_pipe.h"
#include <tbox/base/defines.h>

#include <cstring>
#include <cassert>

#include <vector>
#include <deque>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace tbox {
namespace util {

using namespace std;

class AsyncPipe::Impl {
  public:
    class Buffer {
      public:
        Buffer(size_t cap);
        ~Buffer();

        NONCOPYABLE(Buffer);
        IMMOVABLE(Buffer);

      public:
        size_t append(const void *data_ptr, size_t data_size);

        inline bool   full()  const { return capacity_ == size_; }
        inline bool   empty() const { return size_ == 0; }
        inline void  *data()  const { return data_; }
        inline size_t size()  const { return size_; }
        inline void   reset() { size_ = 0; }

      private:
        size_t  capacity_;
        size_t  size_ = 0;
        uint8_t *data_ = nullptr;
    };

  public:
    Impl();
    ~Impl();

    bool initialize(const Config &cfg);
    void setCallback(const Callback &cb) { cb_ = cb; }
    void cleanup();
    void append(const void *data_ptr, size_t data_size);
    void appendLock();
    void appendUnlock();
    void appendLockless(const void *data_ptr, size_t data_size);

  protected:
    void threadFunc();

  private:
    Config      cfg_;
    Callback    cb_;

    Buffer*         curr_buffer_ = nullptr; //!< 当前缓冲
    vector<Buffer*> free_buffers_;  //!< 可用缓冲数组
    deque<Buffer*>  full_buffers_;  //!< 已满缓冲队列
    size_t          buff_num_;      //!< 缓冲个数

    bool    inited_ = false;        //!< 是否已经启动子线程
    bool    stop_signal_ = false;   //!< 停止信号
    thread  backend_thread_;

    mutex   curr_buffer_mutex_;     //!< 锁 curr_buffer_ 的
    mutex   full_buffers_mutex_;    //!< 锁 full_buffers_ 的
    mutex   free_buffers_mutex_;    //!< 锁 free_buffers_ 的
    mutex   buff_num_mutex_;        //!< 锁 buff_num_ 的
    condition_variable full_buffers_cv_;    //!< full_buffers_ 不为空条件变量
    condition_variable free_buffers_cv_;    //!< free_buffers_ 不为空条件变量
};

AsyncPipe::Impl::Buffer::Buffer(size_t cap) :
    capacity_(cap)
{
    data_ = new uint8_t [cap];
}

AsyncPipe::Impl::Buffer::~Buffer()
{
    delete [] data_;
}

size_t AsyncPipe::Impl::Buffer::append(const void *data_ptr, size_t data_size)
{
    size_t wsize = data_size;
    if ((size_ + data_size) > capacity_)
        wsize = capacity_ - size_;

    ::memcpy((data_ + size_), data_ptr, wsize);
    size_ += wsize;

    return wsize;
}

AsyncPipe::AsyncPipe() :
    impl_(new Impl)
{ }

AsyncPipe::~AsyncPipe()
{
    delete impl_;
}

bool AsyncPipe::initialize(const Config &cfg)
{
    return impl_->initialize(cfg);
}

void AsyncPipe::setCallback(const Callback &cb)
{
    impl_->setCallback(cb);
}

void AsyncPipe::cleanup()
{
    impl_->cleanup();
}

void AsyncPipe::append(const void *data_ptr, size_t data_size)
{
    impl_->append(data_ptr, data_size);
}

void AsyncPipe::appendLock()
{
    impl_->appendLock();
}

void AsyncPipe::appendUnlock()
{
    impl_->appendUnlock();
}

void AsyncPipe::appendLockless(const void *data_ptr, size_t data_size)
{
    impl_->appendLockless(data_ptr, data_size);
}

AsyncPipe::Impl::Impl()
{ }

AsyncPipe::Impl::~Impl()
{
    cleanup();
}

bool AsyncPipe::Impl::initialize(const Config &cfg)
{
    if (cfg.buff_size == 0) {
        std::cerr << "Err: AsyncPipe::Config::buff_size == 0" << std::endl;
        return false;
    }

    if (cfg.buff_min_num == 0) {
        std::cerr << "Err: AsyncPipe::Config::buff_min_num == 0" << std::endl;
        return false;
    }

    if (cfg.buff_min_num > cfg.buff_max_num) {
        std::cerr << "Err: AsyncPipe::Config::buff_max_num < buff_min_num" << std::endl;
        return false;
    }

    if (cfg.interval == 0) {
        std::cerr << "Err: AsyncPipe::Config::interval == 0" << std::endl;
        return false;
    }

    cfg_ = cfg;

    free_buffers_.reserve(cfg.buff_min_num);
    for (size_t i = 0; i < cfg.buff_min_num; ++i)
        free_buffers_.push_back(new Buffer(cfg.buff_size));
    buff_num_ = cfg.buff_min_num;

    auto bt = thread(std::bind(&AsyncPipe::Impl::threadFunc, this));
    backend_thread_.swap(bt);
    inited_ = true;
    return true;
}

void AsyncPipe::Impl::cleanup()
{
    if (!inited_)
        return;

    stop_signal_ = true;
    full_buffers_cv_.notify_all();
    backend_thread_.join();
    stop_signal_ = false;

    assert(full_buffers_.empty());
    CHECK_DELETE_RESET_OBJ(curr_buffer_);
    for (auto item : free_buffers_)
        CHECK_DELETE_RESET_OBJ(item);
    free_buffers_.clear();

    cb_ = nullptr;
    inited_ = false;
}

void AsyncPipe::Impl::append(const void *data_ptr, size_t data_size)
{
    std::lock_guard<std::mutex> lg(curr_buffer_mutex_);
    appendLockless(data_ptr, data_size);
}

void AsyncPipe::Impl::appendLock()
{
    curr_buffer_mutex_.lock();
}

void AsyncPipe::Impl::appendUnlock()
{
    curr_buffer_mutex_.unlock();
}

void AsyncPipe::Impl::appendLockless(const void *data_ptr, size_t data_size)
{
    const uint8_t *ptr = static_cast<const uint8_t*>(data_ptr);
    size_t  remain_size = data_size;

    while (remain_size > 0) {
        if (curr_buffer_ == nullptr) {
            //! 如果 curr_buffer_ 没有分配，则应该从 free_buffers_ 中取一个出来
            std::unique_lock<std::mutex> lk(free_buffers_mutex_);
            if (free_buffers_.empty()) {    //! 如里 free_buffers_ 为空
                buff_num_mutex_.lock();
                //! 如果缓冲块数还没有达到最大限值，则可以继续申请
                if (buff_num_ < cfg_.buff_max_num) {
                    ++buff_num_;
                    buff_num_mutex_.unlock();
                    free_buffers_.push_back(new Buffer(cfg_.buff_size));
                } else {  //! 否则只能等待后端释放
                    buff_num_mutex_.unlock();
                    free_buffers_cv_.wait(lk, [this] { return !free_buffers_.empty(); });
                }
            }

            //! 将 free_buffers_ 中最后的一个弹出来，给到 curr_buffer_
            curr_buffer_ = free_buffers_.back();
            free_buffers_.pop_back();
            //! Q: 为什么从 free_buffers_ 尾部取，而不是向 full_buffers_ 那样从头部取呢？
            //! A: 因为 free_buffers_ 所存空闲缓冲，没有顺序要求。而 full_buffers_ 必须要有顺序性
            //!    既然不需要顺序性，那么 vector 的尾部进出是最高效的。
        }
        auto size = curr_buffer_->append(ptr, remain_size);
        if (curr_buffer_->full()) {
            //! 如果当前缓冲满了
            std::lock_guard<std::mutex> lg2(full_buffers_mutex_);
            full_buffers_.push_back(curr_buffer_);  //! 将 curr_buffer_ 放到 full_buffers_ 中
            full_buffers_cv_.notify_all();  //! 通知后台线程开始干活
            curr_buffer_ = nullptr;
        }
        ptr += size;
        remain_size -= size;
    }
}

void AsyncPipe::Impl::threadFunc()
{
    for (;;) {
        bool is_wake_for_quit = false;  //! 是否因需要停止而被唤醒
        bool is_wake_for_timeup = true; //! 是否因超时而被唤醒

        {
            //! 等待唤醒信号
            std::unique_lock<std::mutex> lk(full_buffers_mutex_);
            if (full_buffers_.empty()) {
                //! 等待三种情况: 1.超时，2.停止，3.full_buffers_不为空
                full_buffers_cv_.wait_for(lk, std::chrono::milliseconds(cfg_.interval),
                    [this, &is_wake_for_timeup, &is_wake_for_quit] {
                        if (stop_signal_)
                            is_wake_for_quit = true;

                        if (is_wake_for_quit || !full_buffers_.empty()) {
                            is_wake_for_timeup = false;
                            return true;
                        }
                        return false;
                    }
                );
            } else {
                is_wake_for_timeup = false;
            }
        }

        //! 如果是超时或是收到停止信号，则先将 curr_buff_ 移到 full_buffers_
        if (is_wake_for_timeup || is_wake_for_quit) {
            if (curr_buffer_mutex_.try_lock()) {
                if (curr_buffer_ != nullptr) {
                    //! Q: 这里为什么不锁 full_buffers_mutex_ ?
                    //! A: 因为锁住了 curr_buffer_mutex_ 就不会有前端调用 appendLockless()，仅有后端的线程操作。
                    //!    所以不锁 full_buffers_mutex_ 也是安全的
                    full_buffers_.push_back(curr_buffer_);
                    curr_buffer_ = nullptr;
                }
                curr_buffer_mutex_.unlock();
            }
        }

        //! 然后逐一处理 full_buffers_ 中的数据
        for (;;) {
            Buffer *buff = nullptr;
            {
                //! 尝试从 full_buffers_ 中取一个出来
                std::lock_guard<std::mutex> lg(full_buffers_mutex_);
                if (!full_buffers_.empty()) {
                    buff = full_buffers_.front();
                    full_buffers_.pop_front();
                } else {
                    break;
                }
            }

            if (buff != nullptr) {
                //! 进行处理
                if (cb_)
                    cb_(buff->data(), buff->size());
                buff->reset();

                buff_num_mutex_.lock();
                if (buff_num_ > cfg_.buff_min_num) {
                    --buff_num_;
                    buff_num_mutex_.unlock();
                    delete buff;
                } else {
                    buff_num_mutex_.unlock();
                    //! 将处理后的缓冲放回 free_buffers_ 中
                    std::lock_guard<std::mutex> lg(free_buffers_mutex_);
                    free_buffers_.push_back(buff);
                    free_buffers_cv_.notify_all();
                }
            }
        }

        if (is_wake_for_quit)
            break;
    }
}

}
}
