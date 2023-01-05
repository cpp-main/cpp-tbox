#ifndef TBOX_MAIN_EXAMPLE_SAMPLE_H_20211226
#define TBOX_MAIN_EXAMPLE_SAMPLE_H_20211226

#include <tbox/main/main.h>
#include <tbox/event/timer_event.h>

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

  private:
    tbox::event::TimerEvent *timer_;
};

#endif //TBOX_MAIN_EXAMPLE_SAMPLE_H_20211226
