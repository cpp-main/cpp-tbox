#ifndef TBOX_MAIN_APPS_IMP_H_20220116
#define TBOX_MAIN_APPS_IMP_H_20220116

#include <vector>

#include "apps.h"
#include <tbox/base/json_fwd.h>

namespace tbox::main {

class App;
class Context;

class AppsImp : public Apps {
  public:
    virtual ~AppsImp();

    enum class State {
        kNone,
        kConstructed,
        kInited,
        kRunning,
    };

  public:
    bool add(App *app) override;
    bool empty() const;

    void fillDefaultConfig(Json &cfg) const;

    bool construct(Context &ctx);
    bool initialize(const Json &cfg);
    bool start();
    void stop();
    void cleanup();

  private:
    std::vector<App*> apps_;
    State state_ = State::kNone;
};

}

#endif //TBOX_MAIN_APPS_IMP_H_20220116
