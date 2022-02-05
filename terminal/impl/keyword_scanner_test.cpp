#include <gtest/gtest.h>

#include "keyword_scanner.h"

using namespace tbox::terminal;

TEST(KeywordScanner, Prinable)
{
    KeywordScanner ks;
    ks.start();
    EXPECT_EQ(ks.next('a'), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('z'), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('A'), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('Z'), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('9'), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('0'), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next(' '), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPrintable);

    ks.start();
    EXPECT_EQ(ks.next('~'), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPrintable);
}

TEST(KeywordScanner, Tab)
{
    KeywordScanner ks;
    EXPECT_EQ(ks.next(0x09), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kTab);
}

TEST(KeywordScanner, Backspace)
{
    KeywordScanner ks;
    EXPECT_EQ(ks.next(0x7f), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kBackspace);
}

TEST(KeywordScanner, Esc)
{
    KeywordScanner ks;
    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.stop(), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kESC);
}

TEST(KeywordScanner, Enter)
{
    KeywordScanner ks;
    EXPECT_EQ(ks.next(0x0d), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x00), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kEnter);
}

TEST(KeywordScanner, Alt)
{
    KeywordScanner ks;

    ks.start();
    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(' '), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kAltPlus);
    EXPECT_EQ(ks.extra(), ' ');

    ks.start();
    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next('~'), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kAltPlus);
    EXPECT_EQ(ks.extra(), '~');
}

TEST(KeywordScanner, CtrlAlt)
{
    KeywordScanner ks;

    ks.start();
    EXPECT_EQ(ks.next(0xc2), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next('a' + 0x20), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kCtrlAltPlus);
    EXPECT_EQ(ks.extra(), 'a');

    ks.start();
    EXPECT_EQ(ks.next(0xc2), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next('z' + 0x20), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kCtrlAltPlus);
    EXPECT_EQ(ks.extra(), 'z');
}

TEST(KeywordScanner, MoveUp)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x41), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kMoveUp);
}

TEST(KeywordScanner, MoveDown)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x42), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kMoveDown);
}

TEST(KeywordScanner, MoveRight)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x43), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kMoveRight);
}

TEST(KeywordScanner, MoveLeft)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x44), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kMoveLeft);
}

TEST(KeywordScanner, Home)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kHome);
}

TEST(KeywordScanner, Insert)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kInsert);
}

TEST(KeywordScanner, Delete)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x33), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kDelete);
}

TEST(KeywordScanner, End)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x34), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kEnd);
}

TEST(KeywordScanner, PageUp)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x35), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPageUp);
}

TEST(KeywordScanner, PageDown)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x36), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kPageDown);
}

TEST(KeywordScanner, F1)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x4f), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x50), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF1);
}

TEST(KeywordScanner, F2)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x4f), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x51), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF2);
}

TEST(KeywordScanner, F3)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x4f), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x52), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF3);
}

TEST(KeywordScanner, F4)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x4f), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x53), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF4);
}

TEST(KeywordScanner, F5)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x35), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF5);
}

TEST(KeywordScanner, F6)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x37), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF6);
}

TEST(KeywordScanner, F7)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x38), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF7);
}

TEST(KeywordScanner, F8)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x39), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF8);
}

TEST(KeywordScanner, F9)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x30), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF9);
}

TEST(KeywordScanner, F10)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x31), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF10);
}

TEST(KeywordScanner, F11)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x33), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF11);
}

TEST(KeywordScanner, F12)
{
    KeywordScanner ks;
    ks.start();

    EXPECT_EQ(ks.next(0x1b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x5b), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x32), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x34), KeywordScanner::Status::kUnsure);
    EXPECT_EQ(ks.next(0x7e), KeywordScanner::Status::kEnsure);
    EXPECT_EQ(ks.result(), KeywordScanner::Result::kF12);
}

