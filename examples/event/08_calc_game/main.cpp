/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
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
    cout << proc_name << " epoll" << endl;
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
