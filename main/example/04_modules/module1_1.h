#ifndef MODULE1_1_H_20220328
#define MODULE1_1_H_20220328

#include <tbox/main/module.h>

class Module1_1 : public tbox::main::Module {
  public:
    Module1_1(tbox::main::Context &ctx);

    virtual bool initialize(const Json &js) override;
    virtual bool start() override;
    virtual void stop() override;
    virtual void cleanup() override;
};

#endif //MODULE1_1_H_20220328
