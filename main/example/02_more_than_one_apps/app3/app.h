#ifndef TBOX_MAIN_EXAMPLE_APP3_H_20211226
#define TBOX_MAIN_EXAMPLE_APP3_H_20211226

#include <tbox/main/main.h>

namespace app3 {

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

#endif //TBOX_MAIN_EXAMPLE_APP3_H_20211226
