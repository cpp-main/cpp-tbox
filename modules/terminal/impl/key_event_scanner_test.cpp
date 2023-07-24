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
#include <gtest/gtest.h>

#include "key_event_scanner.h"

using namespace tbox::terminal;

TEST(KeyEventScanner, Printable)
{
    KeyEventScanner ks;
    ks.start();
    EXPECT_EQ(ks.next('a'), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('z'), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('A'), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('Z'), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('9'), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('0'), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next(' '), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('~'), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPrintable);
}

TEST(KeyEventScanner, Tab)
{
    KeyEventScanner ks;
    EXPECT_EQ(ks.next(0x09), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kTab);
}

TEST(KeyEventScanner, Backspace_1)
{
    KeyEventScanner ks;
    EXPECT_EQ(ks.next(0x7f), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kBackspace);
}

TEST(KeyEventScanner, Backspace_2)
{
    KeyEventScanner ks;
    EXPECT_EQ(ks.next(0x08), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kBackspace);
}

TEST(KeyEventScanner, Esc)
{
    KeyEventScanner ks;
    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.stop(), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kESC);
}

TEST(KeyEventScanner, Enter_1)
{
    KeyEventScanner ks;
    EXPECT_EQ(ks.next(0x0d), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x00), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kEnter);
}

TEST(KeyEventScanner, Enter_2)
{
    KeyEventScanner ks;
    EXPECT_EQ(ks.next(0x0d), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x0a), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kEnter);
}

TEST(KeyEventScanner, Enter_3)
{
    KeyEventScanner ks;
    EXPECT_EQ(ks.next(0x0d), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.stop(), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kEnter);
}

TEST(KeyEventScanner, Enter_4)
{
    KeyEventScanner ks;
    EXPECT_EQ(ks.next(0x0a), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kEnter);
}

TEST(KeyEventScanner, Alt)
{
    KeyEventScanner ks;

    ks.start();
    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(' '), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kAltPlus);
    EXPECT_EQ(ks.extra(), ' ');

    ks.start();
    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next('~'), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kAltPlus);
    EXPECT_EQ(ks.extra(), '~');
}

TEST(KeyEventScanner, CtrlAlt)
{
    KeyEventScanner ks;

    ks.start();
    EXPECT_EQ(ks.next(0xc2), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next('a' + 0x20), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kCtrlAltPlus);
    EXPECT_EQ(ks.extra(), 'a');

    ks.start();
    EXPECT_EQ(ks.next(0xc2), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next('z' + 0x20), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kCtrlAltPlus);
    EXPECT_EQ(ks.extra(), 'z');
}

TEST(KeyEventScanner, MoveUp)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x41), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kMoveUp);
}

TEST(KeyEventScanner, MoveDown)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x42), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kMoveDown);
}

TEST(KeyEventScanner, MoveRight)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x43), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kMoveRight);
}

TEST(KeyEventScanner, MoveLeft)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x44), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kMoveLeft);
}

TEST(KeyEventScanner, Home)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kHome);
}

TEST(KeyEventScanner, Insert)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kInsert);
}

TEST(KeyEventScanner, Delete)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x33), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kDelete);
}

TEST(KeyEventScanner, End)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x34), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kEnd);
}

TEST(KeyEventScanner, PageUp)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x35), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPageUp);
}

TEST(KeyEventScanner, PageDown)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x36), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kPageDown);
}

TEST(KeyEventScanner, F1)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x4f), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x50), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF1);
}

TEST(KeyEventScanner, F2)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x4f), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x51), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF2);
}

TEST(KeyEventScanner, F3)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x4f), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x52), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF3);
}

TEST(KeyEventScanner, F4)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x4f), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x53), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF4);
}

TEST(KeyEventScanner, F5)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x35), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF5);
}

TEST(KeyEventScanner, F6)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x37), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF6);
}

TEST(KeyEventScanner, F7)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x38), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF7);
}

TEST(KeyEventScanner, F8)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x39), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF8);
}

TEST(KeyEventScanner, F9)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x30), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF9);
}

TEST(KeyEventScanner, F10)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF10);
}

TEST(KeyEventScanner, F11)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x33), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF11);
}

TEST(KeyEventScanner, F12)
{
    KeyEventScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x34), KeyEventScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeyEventScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeyEventScanner::Result::kF12);
}

