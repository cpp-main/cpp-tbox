#include "async_data_stream.h"
#include <tbox/base/defines.h>

#include <cstring>
#include <vector>
#include <list>

namespace tbox {
namespace util {

using namespace std;

class AsyncDataStream::Impl {
  public:
    class Buffer {
      public:
        Buffer(size_t cap);
        ~Buffer();

        NONCOPYABLE(Buffer);
        IMMOVABLE(Buffer);

      public:
        size_t append(const void *data_ptr, size_t data_size);

        inline void *data() const { return data_; }
        inline size_t size() const { return size_; }
        inline size_t capacity() const { return capacity_; }

      private:
        size_t capacity_;
        uint8_t *data_ = nullptr;
        size_t size_ = 0;
    };

  private:
    Buffer* curr_buffer_ = nullptr; //!< 当前缓冲
    vector<Buffer*> empty_buffers_; //!< 空缓冲
    list<Buffer*>   full_buffers_;  //!< 已满缓冲
};

AsyncDataStream::Impl::Buffer::Buffer(size_t cap) :
    capacity_(cap)
{
    data_ = new uint8_t [cap];
}

AsyncDataStream::Impl::Buffer::~Buffer()
{
    delete data_;
}

size_t AsyncDataStream::Impl::Buffer::append(const void *data_ptr, size_t data_size)
{
    size_t wsize = data_size;
    if ((size_ + data_size) > capacity_)
        wsize = capacity_ - size_;

    ::memcpy((data_ + size_), data_ptr, wsize);
    size_ += wsize;

    return wsize;
}

AsyncDataStream::AsyncDataStream()
{
    //!TODO
}

AsyncDataStream::~AsyncDataStream()
{
    //!TODO
}

bool AsyncDataStream::initialize(const Config &cfg)
{
    //!TODO
    return false;
}

void AsyncDataStream::cleanup()
{
    //!TODO
}

void AsyncDataStream::asyncWrite(const void *data_ptr, size_t data_size)
{
    //!TODO
}

}
}
