#ifndef TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226
#define TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226

#include <tbox/main/main.h>
#include <tbox/network/tcp_server.h>

namespace echo_server {

using namespace tbox;

class App : public main::Module
{
  public:
    App(main::Context &ctx);
    ~App();

    void onFillDefaultConfig(Json &cfg) override;
    bool onInitialize(const tbox::Json &cfg) override;
    bool onStart() override;
    void onStop() override;
    void onCleanup() override;

  private:
    network::TcpServer *server_ = nullptr;
};

}

#endif //TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226
