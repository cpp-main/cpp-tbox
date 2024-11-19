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
#include "uart.h"

#include <termios.h>
#include <fcntl.h>

#include <tbox/base/log.h>

namespace tbox {
namespace network {

Uart::Uart(event::Loop *wp_loop) :
    buff_fd_(wp_loop)
{ }

bool Uart::initialize(const std::string &dev, const std::string &mode)
{
    if (!openDevice(dev))
        return false;
    return setMode(mode);
}

bool Uart::initialize(const std::string &dev, const Mode &mode)
{
    if (!openDevice(dev))
        return false;
    return setMode(mode);
}

namespace {
int Baudrate2Enum(int baudrate)
{
    switch (baudrate) {
#ifdef B50
        case (50): return B50;
#endif /* B50 */
#ifdef B75
        case (75): return B75;
#endif /* B75 */
#ifdef B110
        case (110): return B110;
#endif /* B110 */
#ifdef B134
        case (134): return B134;
#endif /* B134 */
#ifdef B150
        case (150): return B150;
#endif /* B150 */
#ifdef B200
        case (200): return B200;
#endif /* B200 */
#ifdef B300
        case (300): return B300;
#endif /* B300 */
#ifdef B600
        case (600): return B600;
#endif /* B600 */
#ifdef B1200
        case (1200): return B1200;
#endif /* B1200 */
#ifdef B1800
        case (1800): return B1800;
#endif /* B1800 */
#ifdef B2400
        case (2400): return B2400;
#endif /* B2400 */
#ifdef B4800
        case (4800): return B4800;
#endif /* B4800 */
#ifdef B9600
        case (9600): return B9600;
#endif /* B9600 */
#ifdef B19200
        case (19200): return B19200;
#endif /* B19200 */
#ifdef B38400
        case (38400): return B38400;
#endif /* B38400 */
#ifdef B57600
        case (57600): return B57600;
#endif /* B57600 */
#ifdef B115200
        case (115200): return B115200;
#endif /* B115200 */
#ifdef B230400
        case (230400): return B230400;
#endif /* B230400 */
#ifdef B345600
        case (345600): return B345600;
#endif /* B345600 */
#ifdef B460800
        case (460800): return B460800;
#endif /* B460800 */
#ifdef B500000
        case (500000): return B500000;
#endif /* B500000 */
#ifdef B576000
        case (576000): return B576000;
#endif /* B576000 */
#ifdef B921600
        case (921600): return B921600;
#endif /* B921600 */
#ifdef B1000000
        case (1000000): return B1000000;
#endif /* B1000000 */
#ifdef B1152000
        case (1152000): return B1152000;
#endif /* B1152000 */
#ifdef B1500000
        case (1500000): return B1500000;
#endif /* B1500000 */
#ifdef B2000000
        case (2000000): return B2000000;
#endif /* B2000000 */
#ifdef B2500000
        case (2500000): return B2500000;
#endif /* B2500000 */
#ifdef B3000000
        case (3000000): return B3000000;
#endif /* B3000000 */
#ifdef B3500000
        case (3500000): return B3500000;
#endif /* B3500000 */
#ifdef B4000000
        case (4000000): return B4000000;
#endif /* B4000000 */
        default:
            return -1;
    }
}

}

bool Uart::setMode(const Mode &mode)
{
    if (fd_.get() < 0) {
        LogErr("please open first");
        return false;
    }

    int baud = Baudrate2Enum(mode.baudrate);
    if (baud == -1) {
        LogErr("baudrate %d not support", mode.baudrate);
        return false;
    }

    struct termios options;
    if (tcgetattr(fd_.get(), &options) == -1) {
        LogErr("tcgetattr fail, errno:%d", errno);
        return false;
    }

    options.c_iflag &= ~(INLCR | IGNCR | ICRNL | IUCLC | IXON | IXANY | IXOFF);
    options.c_iflag |= IGNBRK | IGNPAR;
    options.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
    options.c_cflag &= ~(CRTSCTS);
    options.c_cflag |= CREAD | HUPCL | CLOCAL;
    options.c_lflag &= ~(ISIG | ICANON | ECHO | IEXTEN);

    options.c_cflag &= ~CSIZE;
    if (mode.data_bit == DataBit::k8bits)
        options.c_cflag |= CS8;
    else
        options.c_cflag |= CS7;

    options.c_cflag &= ~PARENB;
    options.c_iflag &= ~(INPCK | ISTRIP);
    if (mode.parity != ParityEnd::kNoEnd) {
        options.c_cflag |= PARENB;
        options.c_iflag |= (INPCK | ISTRIP);
        if (mode.parity == ParityEnd::kOddEnd)
            options.c_cflag |= PARODD;
        else
            options.c_cflag &= ~PARODD;
    }

    if (mode.stop_bit == StopBit::k1bits)
        options.c_cflag &= ~CSTOPB;
    else
        options.c_cflag |= CSTOPB;

    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    if (tcsetattr(fd_.get(), TCSAFLUSH, &options) == -1) {
        LogErr("tcsetattr fail, errno:%d", errno);
        return false;
    }

    return true;
}

//! 解析如："115200 8n1" 这样的字串
bool Uart::setMode(const std::string &mode_str)
{
    using namespace std;

    if (mode_str.empty()) {
        LogErr("mode_str is empty");
        return false;
    }

    size_t pos = mode_str.find_first_of(' ', 0);
    if ((pos == string::npos) || ((pos + 4) < mode_str.size())) {
        LogErr("mode_str format invalid");
        return false;
    }

    //! 提取各字段
    string baudrate_str = mode_str.substr(0, pos);
    char data_bits_ch = mode_str[pos + 1];
    char parity_ch = toupper(mode_str[pos + 2]);
    char stop_bits_ch = mode_str[pos + 3];

    Mode mode;

    //! 波特率
    mode.baudrate = stoi(baudrate_str);

    //! 数据位
    if (data_bits_ch == '8')
        mode.data_bit = DataBit::k8bits;
    else if (data_bits_ch == '7')
        mode.data_bit = DataBit::k7bits;
    else {
        LogErr("unsupport databit %c", data_bits_ch);
        return false;
    }

    //! 奇偶校验位
    if (parity_ch == 'N')
        mode.parity = ParityEnd::kNoEnd;
    else if (parity_ch == 'O')
        mode.parity = ParityEnd::kOddEnd;
    else if (parity_ch == 'E')
        mode.parity = ParityEnd::kEvenEnd;
    else {
        LogErr("unsupport parity %c", parity_ch);
        return false;
    }

    //! 停止位
    if (stop_bits_ch == '1')
        mode.stop_bit = StopBit::k1bits;
    else if (stop_bits_ch == '2')
        mode.stop_bit = StopBit::k2bits;
    else {
        LogErr("unsupport stopbit %c", stop_bits_ch);
        return false;
    }

    return setMode(mode);
}

void Uart::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    buff_fd_.setReceiveCallback(cb, threshold);
}

void Uart::setSendCompleteCallback(const SendCompleteCallback &cb)
{
    buff_fd_.setSendCompleteCallback(cb);
}

bool Uart::send(const void *data_ptr, size_t data_size)
{
    return buff_fd_.send(data_ptr, data_size);
}

void Uart::bind(ByteStream *receiver)
{
    buff_fd_.bind(receiver);
}

void Uart::unbind()
{
    buff_fd_.unbind();
}

Buffer* Uart::getReceiveBuffer()
{
    return buff_fd_.getReceiveBuffer();
}

bool Uart::enable()
{
    if (fd_.isNull()) {
        LogErr("please initialize first");
        return false;
    }

    return buff_fd_.enable();
}

bool Uart::disable()
{
    if (fd_.isNull()) {
        LogErr("please initialize first");
        return false;
    }

    return buff_fd_.disable();
}

bool Uart::openDevice(const std::string &dev)
{
    Fd fd = Fd::Open(dev.c_str(), O_RDWR | O_NOCTTY);
    if (fd.isNull()) {
        LogErr("Uart open fail");
        return false;
    }

    fd_ = std::move(fd);
    return buff_fd_.initialize(fd_);
}

}
}
