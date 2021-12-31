#ifndef TBOX_MAIN_APP_H_20211222
#define TBOX_MAIN_APP_H_20211222

#include <tbox/base/json_fwd.h>

namespace tbox::main {

//! 应用
class App {
  public:
    virtual ~App() {}

    virtual void fillDefaultConfig(Json &cfg) const { }

    virtual bool initialize(const Json &cfg) = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;
};

}

#endif //TBOX_MAIN_APP_H_20211222
