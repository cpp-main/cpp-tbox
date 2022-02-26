#include <iostream>
#include <tbox/event/loop.h>

#ifdef USE_LITE
    #include "game_lite.h"
#else
    #include "game.h"
#endif

using namespace std;
void PrintUsage(const char *proc_name)
{
    cout << proc_name << " <libevent|libev>" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    using namespace tbox::event;

    Loop* sp_loop = Loop::New(argv[1]);
    if (sp_loop == nullptr) {
        PrintUsage(argv[0]);
        return 0;
    }

#ifdef USE_LITE
    GameLite game_lite;
    game_lite.init(sp_loop);
#else
    Game game;
    game.init(sp_loop);
#endif

    sp_loop->runLoop(Loop::Mode::kForever);

#ifdef USE_LITE
    game_lite.cleanup();
#else
    game.cleanup();
#endif

    delete sp_loop;

    return 0;
}
