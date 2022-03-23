#ifndef TBOX_MAIN_EXAMPLE_APP2_H_20211226
#define TBOX_MAIN_EXAMPLE_APP2_H_20211226

#include <tbox/main/main.h>

namespace app2 {

class App : public tbox::main::App
{
  public:
    ~App();

    virtual bool construct(tbox::main::Context &ctx) override;
    virtual bool initialize(const tbox::Json &cfg) override;
    virtual bool start() override;
    virtual void stop() override;
    virtual void cleanup() override;
};

}

#endif //TBOX_MAIN_EXAMPLE_APP2_H_20211226
