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
    LogUndo();
}

bool BufferedFd::enable()
{
    LogUndo();
    return false;
}

bool BufferedFd::disable()
{
    LogUndo();
    return false;
}

bool BufferedFd::send(const void *p_data, size_t data_size)
{
    LogUndo();
    return false;
}

void BufferedFd::onReadCallback(short)
{
    LogUndo();
}

void BufferedFd::onWriteCallback(short)
{
    LogUndo();
}

}
}
