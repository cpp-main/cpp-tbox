#ifndef TBOX_MAIN_EXAMPLE_APP1_H_20211226
#define TBOX_MAIN_EXAMPLE_APP1_H_20211226

#include <tbox/main/main.h>

namespace app1 {

class App : public tbox::main::App
{
  public:
    ~App();

    bool construct(tbox::main::Context &ctx) override;
    bool initialize(const tbox::Json &cfg) override;
    bool start() override;
    void stop() override;
    void cleanup() override;
};

}

#endif //TBOX_MAIN_EXAMPLE_APP1_H_20211226
