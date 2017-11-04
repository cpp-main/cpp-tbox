#ifndef TBOX_NETWORK_UART_H_20171104
#define TBOX_NETWORK_UART_H_20171104

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include "byte_stream.h"
#include "buffered_fd.h"

namespace tbox {
namespace network {

class Uart : public ByteStream {
  public: 
    explicit Uart(event::Loop *wp_loop);
    virtual ~Uart();

    NONCOPYABLE(Uart);
    IMMOVABLE(Uart);

  public:
    enum class DataBit { k7bits, k8bits };  //! 数据位数
    enum class StopBit { k1bits, k2bits };  //! 停止位数
    enum class ParityEnd { kNoEnd, kOddEnd, kEvenEnd }; //! 检验位

    bool open(const std::string &dev);
    Fd fd() const { return fd_; }

    bool initialize(int baudrate, DataBit data_bit, ParityEnd parity, StopBit stop_bit);

  public:
    //! 实现ByteStream的接口
    void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    bool send(const void *data_ptr, size_t data_size) override;
    void bind(ByteStream *receiver) override;
    void unbind() override;

  public:
    bool enable();
    bool disable();

  private:
    Fd fd_;
    BufferedFd buff_fd_;
};

}
}

#endif //TBOX_NETWORK_UART_H_20171104
