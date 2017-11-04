#include "uart.h"

#include <tbox/base/log.h>

namespace tbox {
namespace network {

Uart::Uart(event::Loop *wp_loop) :
    buff_fd_(wp_loop)
{
    LogUndo();
}

Uart::~Uart()
{
    LogUndo();
}

bool Uart::open(const std::string &dev)
{
    LogUndo();
    return false;
}

bool Uart::initialize(int baudrate, DataBit data_bit, ParityEnd parity, StopBit stop_bit)
{
    LogUndo();
    return false;
}

void Uart::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    LogUndo();
}

bool Uart::send(const void *data_ptr, size_t data_size)
{
    LogUndo();
    return false;
}

void Uart::bind(ByteStream *receiver)
{
    LogUndo();
}

void Uart::unbind()
{
    LogUndo();
}

}
}
