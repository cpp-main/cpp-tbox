#include <tbox/main/main.h>

namespace tbox {
    extern void LoadPlugins(int argc, char **argv);
}

int main(int argc, char **argv) {
    tbox::LoadPlugins(argc, argv);
    return tbox::main::Main(argc, argv);
}
