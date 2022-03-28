#ifndef MODULE1_2_H_20220328
#define MODULE1_2_H_20220328

#include <tbox/main/module.h>

class Module1_2 : public tbox::main::Module {
  public:
    Module1_2(tbox::main::Context &ctx);

    virtual bool initialize(const Json &js) override;
    virtual bool start() override;
    virtual void stop() override;
    virtual void cleanup() override;
};

#endif //MODULE1_2_H_20220328
