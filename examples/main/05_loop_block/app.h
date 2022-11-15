#ifndef TBOX_MAIN_EXAMPLE_SAMPLE_H_20211226
#define TBOX_MAIN_EXAMPLE_SAMPLE_H_20211226

#include <tbox/main/main.h>
#include <tbox/eventx/timer_pool.h>

class App : public tbox::main::Module
{
  public:
    App(tbox::main::Context &ctx);

  protected:
    virtual bool onStart() override;
    virtual void onStop() override;

  private:
    tbox::eventx::TimerPool::TimerToken timer_;
};

#endif //TBOX_MAIN_EXAMPLE_SAMPLE_H_20211226
