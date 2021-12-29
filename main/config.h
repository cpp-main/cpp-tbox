#ifndef TBOX_MAIN_CONFIG_H_20211229
#define TBOX_MAIN_CONFIG_H_20211229

namespace tbox::main {

class Config {
  public:
    bool parse(int argc, char **argv);
};

}

#endif //TBOX_MAIN_CONFIG_H_20211229
