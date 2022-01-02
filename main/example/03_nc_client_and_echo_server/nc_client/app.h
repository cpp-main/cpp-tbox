#ifndef TBOX_MAIN_EXAMPLE_NC_CLIENT_H_20211226
#define TBOX_MAIN_EXAMPLE_NC_CLIENT_H_20211226

#include <tbox/main/main.h>
#include <tbox/network/tcp_client.h>
#include <tbox/network/stdio_stream.h>

namespace nc_client {

using namespace tbox;

class App : public main::App
{
  public:
    ~App();

    bool construct(tbox::main::Context &ctx) override;
    bool initialize(const tbox::Json &cfg) override;
    bool start() override;
    void stop() override;
    void cleanup() override;

  private:
    network::TcpClient *client_ = nullptr;
    network::StdioStream *stdio_ = nullptr;
};

}

#endif //TBOX_MAIN_EXAMPLE_NC_CLIENT_H_20211226
