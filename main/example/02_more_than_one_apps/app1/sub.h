#ifndef TBOX_MAIN_EXAMPLE_SUBMODULE_H_20220329
#define TBOX_MAIN_EXAMPLE_SUBMODULE_H_20220329

#include <tbox/main/main.h>

namespace app1 {

class Sub : public tbox::main::Module
{
  public:
    Sub(tbox::main::Context &ctx);
    ~Sub();

  protected:
    virtual bool onInitialize(const tbox::Json &cfg) override;
    virtual bool onStart() override;
    virtual void onStop() override;
    virtual void onCleanup() override;
};

}

#endif //TBOX_MAIN_EXAMPLE_SUBMODULE_H_20220329
