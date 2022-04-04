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

        inline bool   full() const { return capacity_ == size_; }
        inline bool   empty() const { return size_ == 0; }
        inline void  *data() const { return data_; }
        inline size_t size() const { return size_; }
        inline size_t capacity() const { return capacity_; }
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
    void cleanup();
    void append(const void *data_ptr, size_t data_size);

  protected:
    void threadFunc();

  private:
    Config  cfg_;
    Buffer*         curr_buffer_ = nullptr; //!< 当前缓冲
    vector<Buffer*> free_buffers_;  //!< 空缓冲
    deque<Buffer*>  full_buffers_;  //!< 已满缓冲

    bool    inited_ = false;        //! 是否已经启动子线程
    bool    stop_signal_ = false;   //! 停止信号
    thread  backend_thread_;

    mutex   curr_buffer_mutex_;
    mutex   full_buffers_mutex_;
    mutex   free_buffers_mutex_;
    condition_variable full_buffers_cv_;
    condition_variable free_buffers_cv_;
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

void AsyncPipe::cleanup()
{
    impl_->cleanup();
}

void AsyncPipe::append(const void *data_ptr, size_t data_size)
{
    impl_->append(data_ptr, data_size);
}

AsyncPipe::Impl::Impl()
{ }

AsyncPipe::Impl::~Impl()
{
    cleanup();
}

bool AsyncPipe::Impl::initialize(const Config &cfg)
{
    if (!cfg.cb) {
        std::cerr << "Err: AsyncPipe::Config::cb == null" << std::endl;
        return false;
    }

    if (cfg.buff_size == 0) {
        std::cerr << "Err: AsyncPipe::Config::buff_size == 0" << std::endl;
        return false;
    }

    if (cfg.buff_num == 0) {
        std::cerr << "Err: AsyncPipe::Config::buff_num == 0" << std::endl;
        return false;
    }

    if (cfg.interval == 0) {
        std::cerr << "Err: AsyncPipe::Config::interval == 0" << std::endl;
        return false;
    }

    cfg_ = cfg;
    for (size_t i = 0; i < cfg.buff_num; ++i)
        free_buffers_.push_back(new Buffer(cfg.buff_size));

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

    assert(full_buffers_.empty());
    CHECK_DELETE_RESET_OBJ(curr_buffer_);
    for (auto item : free_buffers_)
        CHECK_DELETE_RESET_OBJ(item);
}

void AsyncPipe::Impl::append(const void *data_ptr, size_t data_size)
{
    const uint8_t *ptr = static_cast<const uint8_t*>(data_ptr);
    size_t  remain_size = data_size;

    std::lock_guard<std::mutex> lg(curr_buffer_mutex_);
    while (remain_size > 0) {
        if (curr_buffer_ == nullptr) {
            //! 如果 curr_buffer_ 没有分配，则应该从 free_buffers_ 中取一个出来
            std::unique_lock<std::mutex> lk(free_buffers_mutex_);
            if (free_buffers_.empty())  //! 如里 free_buffers_ 为空，则要等
                free_buffers_cv_.wait(lk, [this] { return !free_buffers_.empty(); });
            //! 将 free_buffers_ 中最后的一个弹出来，给到 curr_buffer_
            curr_buffer_ = free_buffers_.back();
            free_buffers_.pop_back();
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
    do {
        {
            //! 等待唤醒信号
            std::unique_lock<std::mutex> lk(full_buffers_mutex_);
            //! 等待三种情况: 1.超时，2.停止，3.full_buffers_不为空
            full_buffers_cv_.wait_for(lk, std::chrono::milliseconds(cfg_.interval),
                [=] {
                    return stop_signal_ || !full_buffers_.empty();
                }
            );
        }

        //! 先处理 full_buffers_ 中的数据
        for (;;) {
            Buffer *buff = nullptr;
            {
                //! 尝试从 full_buffers_ 中取一个出来
                std::lock_guard<std::mutex> lg(full_buffers_mutex_);
                if (!full_buffers_.empty()) {
                    buff = full_buffers_.front();
                    full_buffers_.pop_front();
                }
            }

            if (buff != nullptr) {
                //! 进行处理
                cfg_.cb(buff->data(), buff->size());
                buff->reset();
                //! 将处理后的缓冲放回 free_buffers_ 中
                std::lock_guard<std::mutex> lg(free_buffers_mutex_);
                free_buffers_.push_back(buff);
                free_buffers_cv_.notify_all();
            }
        }

        //! 最后检查 curr_buffer_ 中的数据
        {
            Buffer *buff = nullptr;
            {
                std::lock_guard<std::mutex> lg2(curr_buffer_mutex_);
                if (!curr_buffer_->empty()) {
                    buff = curr_buffer_;
                    curr_buffer_ = nullptr;
                }
            }

            if (buff != nullptr) {  //! 如果没取出来
                //! 进行处理
                cfg_.cb(buff->data(), buff->size());
                buff->reset();
                //! 然后将处理后的buff放入到free_buffers_中
                std::lock_guard<std::mutex> lg(free_buffers_mutex_);
                free_buffers_.push_back(buff);
                free_buffers_cv_.notify_all();
            }
        }
    } while (!stop_signal_);  //! 如果是停止信号，则直接跳出循环，结束线程
    /**
     * stop_signal_ 信号为什么不在被唤醒时就break呢？
     * 因为我们期望就算是退出了，Buff中的数据都应该被处理
     */
}

}
}
