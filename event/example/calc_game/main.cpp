#include <tbox/event/loop.h>

#ifdef USE_LITE
    #include "game_lite.h"
#else
    #include "game.h"
#endif

int main()
{
    using namespace tbox::event;

    Loop* sp_loop = Loop::New();

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
