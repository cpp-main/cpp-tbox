#ifndef TBOX_MAIN_APPS_H_20211225
#define TBOX_MAIN_APPS_H_20211225

namespace tbox::main {

class App;

class Apps {
  public:
    virtual bool add(App *app) = 0;
};

}

#endif //TBOX_MAIN_APPS_H_20211225
