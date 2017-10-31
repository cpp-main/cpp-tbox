#include "buffered_fd.h"

#include <cassert>
#include <cstring>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>
#include <tbox/event/fd_item.h>

namespace tbox {
namespace network {

using namespace std::placeholders;

BufferedFd::BufferedFd(event::Loop *wp_loop) :
    wp_loop_(wp_loop),
    send_buff_(0), recv_buff_(0)
{ }

BufferedFd::~BufferedFd()
{
    assert(cb_level_ == 0);

    if (state_ == State::kRunning)
        disable();

    CHECK_DELETE_RESET_OBJ(sp_write_event_);
    CHECK_DELETE_RESET_OBJ(sp_read_event_);
}

bool BufferedFd::initialize(int fd, short events)
{
    if (fd < 0) {
        LogWarn("fd < 0, fd = %d", fd);
        return false;
    }

    if (events == 0) {
        LogWarn("events == 0");
        return false;
    }

    if (state_ != State::kEmpty) {
        LogWarn("can't reinitialize");
        return false;
    }

    fd_ = Fd(fd);
    fd_.setNonBlock(true);

    CHECK_DELETE_RESET_OBJ(sp_write_event_);
    CHECK_DELETE_RESET_OBJ(sp_read_event_);

    if (events & kReadOnly) {
        sp_read_event_ = wp_loop_->newFdItem();
        sp_read_event_->initialize(fd_, event::FdItem::kReadEvent, event::Item::Mode::kPersist);
        sp_read_event_->setCallback(std::bind(&BufferedFd::onReadCallback, this, _1));
    }

    if (events & kWriteOnly) {
        sp_write_event_ = wp_loop_->newFdItem();
        sp_write_event_->initialize(fd_, event::FdItem::kWriteEvent, event::Item::Mode::kPersist);
        sp_write_event_->setCallback(std::bind(&BufferedFd::onWriteCallback, this, _1));
    }

    state_ = State::kInited;

    return true;
}

void BufferedFd::setReceiveCallback(const ReceiveCallback &func, size_t threshold)
{
    receive_cb_ = func;
    receive_threshold_ = threshold;
}

bool BufferedFd::enable()
{
    if (state_ == State::kRunning)
        return true;

    if (state_ != State::kInited) {
        LogWarn("please initialize() first");
        return false;
    }

    if (sp_read_event_ != nullptr)
        sp_read_event_->enable();

    if (sp_write_event_ != nullptr)
        sp_write_event_->enable();

    state_ = State::kRunning;

    return true;
}

bool BufferedFd::disable()
{
    if (state_ == State::kInited)
        return true;

    if (state_ != State::kRunning) {
        LogWarn("please initialize() first");
        return false;
    }

    if (sp_read_event_ != nullptr)
        sp_read_event_->disable();

    if (sp_write_event_ != nullptr)
        sp_write_event_->disable();

    state_ = State::kInited;

    return true;
}

bool BufferedFd::send(const void *data_ptr, size_t data_size)
{
    if (sp_write_event_ == nullptr) {
        LogWarn("send is disabled");
        return false;
    }

    //! 如果当前没有 enable() 或者发送缓冲区中还有没有发送完成的数据
    if ((state_ != State::kRunning) || (send_buff_.readableSize() > 0)) {
        //! 则新的数据就直接放到发送缓冲区
        send_buff_.append(data_ptr, data_size);
    } else {
        //! 否则尝试发送
        ssize_t wsize = fd_.write(data_ptr, data_size);
        if (wsize >= 0) {   //! 如果发送正常
            //! 如果没有发送完，还有剩余的数据
            if (static_cast<size_t>(wsize) < data_size) {
                //! 则将剩余的数据放入到缓冲区
                const uint8_t* p_remain = static_cast<const uint8_t*>(data_ptr) + wsize;
                send_buff_.append(p_remain, (data_size - wsize));
                sp_write_event_->enable();  //! 等待可写事件
            }
        } else {    //! 否则就是出了错
            if (errno == EAGAIN) {  //! 文件操作繁忙
                send_buff_.append(data_ptr, data_size);
                sp_write_event_->enable();  //! 等待可写事件
            } else {
                LogWarn("send fail, some data drop");
                //!TODO
            }
        }
    }

    return true;
}

void BufferedFd::onReadCallback(short)
{
    struct iovec rbuf[2];
    char extbuf[1024];  //! 扩展存储空间

    size_t writable_size = recv_buff_.writableSize();

    //! 优先将数据读入到 recv_buff_ 中去，如果它装不下就再存到 extbuff 中
    rbuf[0].iov_base = recv_buff_.writableBegin();
    rbuf[0].iov_len  = writable_size;
    rbuf[1].iov_base = extbuf;
    rbuf[1].iov_len  = sizeof(extbuf);

    ssize_t rsize = fd_.readv(rbuf, 2);
    if (rsize > 0) {    //! 读到了数据
        do {
            if (static_cast<size_t>(rsize) > writable_size) {
                //! 如果实际读出的数据比 recv_buff_ 的可写区还大，说明有部分数据是写到了 extbuf 中去了
                recv_buff_.hasWritten(writable_size);
                size_t remain_size = rsize - writable_size; //! 计算 extbuf 中的数据大小
                recv_buff_.append(extbuf, remain_size);
            } else {
                recv_buff_.hasWritten(rsize);
            }

            //! 继续读，直到 rsize == 0，表示读完为止
            writable_size = recv_buff_.writableSize();
            rbuf[0].iov_base = recv_buff_.writableBegin();
            rbuf[0].iov_len  = writable_size;
        } while ((rsize = fd_.readv(rbuf, 2)) > 0);

        if (recv_buff_.readableSize() >= receive_threshold_) {
            if (receive_cb_) {
                ++cb_level_;
                receive_cb_(recv_buff_);
                --cb_level_;
            } else {
                LogWarn("receive_cb_ is not set");
                recv_buff_.hasReadAll();    //! 丢弃数据，防止堆积
            }
        }
    } else if (rsize == 0) {    //! 读到0字节数据，说明fd_已不可读了
        sp_read_event_->disable();
        if (read_zero_cb_) {
            ++cb_level_;
            read_zero_cb_();
            --cb_level_;
        }
    } else {    //! 读出错了
        if (errno != EAGAIN) {
            if (error_cb_) {
                ++cb_level_;
                error_cb_(errno);
                --cb_level_;
            } else
                LogWarn("read error, rsize:%d, errno:%d, %s", rsize, errno, strerror(errno));
        }
    }
}

void BufferedFd::onWriteCallback(short)
{
    LogUndo();
}

}
}
