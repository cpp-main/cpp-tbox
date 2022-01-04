#ifndef TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226
#define TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226

#include <tbox/main/main.h>
#include <tbox/network/tcp_server.h>

namespace echo_server {

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
    network::TcpServer *server_ = nullptr;
};

}

#endif //TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226
