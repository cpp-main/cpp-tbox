#ifndef TBOX_MAIN_EXAMPLE_APP1_H_20211226
#define TBOX_MAIN_EXAMPLE_APP1_H_20211226

#include <tbox/main/main.h>

namespace app1 {

class App : public tbox::main::Module
{
  public:
    App(tbox::main::Context &ctx);
    ~App();

  protected:
    virtual bool onInit(const tbox::Json &cfg) override;
    virtual bool onStart() override;
    virtual void onStop() override;
    virtual void onCleanup() override;
};

}

#endif //TBOX_MAIN_EXAMPLE_APP1_H_20211226
