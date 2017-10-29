#include <cassert>
#include <cstring>
#include <utility>

#include "buffer.h"

namespace tbox {
namespace network {

Buffer::Buffer(size_t reverse_size)
{
    //! 如果预留空间大小，那么要为缓冲分配空间
    if (reverse_size > 0) {
        uint8_t *p_buff = new uint8_t [reverse_size];
        assert(p_buff != NULL);
        buffer_ptr_ = p_buff;
        buffer_size_ = reverse_size;
    }
}

Buffer::Buffer(const Buffer &other)
{
    cloneFrom(other);
}

Buffer::~Buffer()
{
    delete [] buffer_ptr_;
    buffer_ptr_ = NULL;
}

void Buffer::swap(Buffer &other)
{
    if (&other == this)
        return;

    std::swap(other.buffer_ptr_,  buffer_ptr_);
    std::swap(other.buffer_size_, buffer_size_);
    std::swap(other.read_index_,  read_index_);
    std::swap(other.write_index_, write_index_);
}

Buffer& Buffer::operator = (const Buffer &other)
{
    if (this != &other)
        cloneFrom(other);

    return *this;
}

void Buffer::reset()
{
    Buffer tmp(0);
    swap(tmp);
}

bool Buffer::ensureWritableSize(size_t write_size)
{
    if (write_size == 0)
        return true;

    //! 空间足够
    if (writableSize() >= write_size)
        return true;

    //! 检查是否可以通过将数据往前挪是否可以满足空间要求
    if ((writableSize() + read_index_) >= write_size) {
        //! 将 readable 区的数据往前移
        ::memmove(buffer_ptr_, (buffer_ptr_ + read_index_), (write_index_ - read_index_));
        write_index_ -= read_index_;
        read_index_ = 0;
        return true;

    } else {    //! 只有重新分配更多的空间才可以
        size_t new_size = (write_index_ + write_size) * 3 / 2;
        uint8_t *p_buff = new uint8_t[new_size];
        if (p_buff == NULL)
            return false;

        if (buffer_ptr_ != NULL) {
            //! 只需要复制 readable 部分数据
            ::memcpy((p_buff + read_index_), (buffer_ptr_ + read_index_), (write_index_ - read_index_));
            delete buffer_ptr_;
        }

        buffer_ptr_  = p_buff;
        buffer_size_ = new_size;

        return true;
    }
}

void Buffer::hasWritten(size_t write_size)
{
    if (write_index_ + write_size > buffer_size_) {
        write_index_ = buffer_size_;
    } else {
        write_index_ += write_size;
    }
}

size_t Buffer::append(const void *p_data, size_t data_size)
{
    if (ensureWritableSize(data_size)) {
        ::memcpy(writableBegin(), p_data, data_size);
        hasWritten(data_size);
        return data_size;
    }
    return 0;
}

void Buffer::hasRead(size_t read_size)
{
    if (read_index_ + read_size > write_index_) {
        read_index_ = write_index_ = 0;
    } else {
        read_index_ += read_size;
        if (read_index_ == write_index_)
            read_index_ = write_index_ = 0;
    }
}

void Buffer::hasReadAll()
{
    read_index_ = write_index_ = 0;
}

size_t Buffer::fetch(void *p_buff, size_t buff_size)
{
    size_t read_size = (buff_size > readableSize()) ? readableSize() : buff_size;
    ::memcpy(p_buff, readableBegin(), read_size);
    hasRead(read_size);
    return read_size;
}

void Buffer::cloneFrom(const Buffer &other)
{
    if (other.buffer_size_ > 0) {
        uint8_t *p_buff = new uint8_t[other.buffer_size_];
        ::memcpy(p_buff, other.buffer_ptr_, other.buffer_size_);
        buffer_ptr_ = p_buff;
    }

    buffer_size_  = other.buffer_size_;
    read_index_   = other.read_index_;
    write_index_  = other.write_index_;
}

}
}
