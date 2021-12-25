#ifndef TBOX_MAIN_APP_H_20211222
#define TBOX_MAIN_APP_H_20211222

namespace tbox::main {

//! 应用
class App {
  public:
    virtual ~App() {}

    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;
};

}

#endif //TBOX_MAIN_APP_H_20211222
