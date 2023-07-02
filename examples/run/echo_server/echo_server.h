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
    virtual ~App() override;

    virtual void onFillDefaultConfig(Json &cfg) override;
    virtual bool onInit(const tbox::Json &cfg) override;
    virtual bool onStart() override;
    virtual void onStop() override;
    virtual void onCleanup() override;

  private:
    network::TcpServer *server_ = nullptr;
};

}

#endif //TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226
