#ifndef TBOX_MAIN_EXAMPLE_NC_CLIENT_H_20211226
#define TBOX_MAIN_EXAMPLE_NC_CLIENT_H_20211226

#include <tbox/main/main.h>
#include <tbox/network/tcp_client.h>
#include <tbox/network/stdio_stream.h>

namespace nc_client {

using namespace tbox;

class App : public main::Module
{
  public:
    App(tbox::main::Context &ctx);
    virtual ~App() override;

    virtual void onFillDefaultConfig(Json &cfg) override;
    virtual bool onInit(const tbox::Json &cfg) override;
    virtual bool onStart() override;
    virtual void onStop() override;
    virtual void onCleanup() override;

  private:
    network::TcpClient *client_ = nullptr;
    network::StdioStream *stdio_ = nullptr;
};

}

#endif //TBOX_MAIN_EXAMPLE_NC_CLIENT_H_20211226
