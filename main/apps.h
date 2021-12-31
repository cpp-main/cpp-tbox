#ifndef TBOX_MAIN_APPS_H_20211225
#define TBOX_MAIN_APPS_H_20211225

#include <tbox/base/json_fwd.h>

namespace tbox::main {

class App;

class Apps {
  public:
    Apps();
    ~Apps();

    enum class State {
        kNone,
        kInited,
        kRunning,
    };

  public:
    bool add(App *app);
    bool empty() const;

    void fillDefaultConfig(Json &cfg) const;

    bool initialize(const Json &cfg);
    bool start();
    void stop();
    void cleanup();

    State state() const;

  private:
    struct Data;
    Data *d_;
};

}

#endif //TBOX_MAIN_APPS_H_20211225
