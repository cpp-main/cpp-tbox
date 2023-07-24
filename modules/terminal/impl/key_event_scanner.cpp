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
#include "key_event_scanner.h"
#include <algorithm>
#include <cctype>

namespace tbox {
namespace terminal {

void KeyEventScanner::start()
{
    result_ = Result::kNone;
    step_ = Step::kNone;
    extra_ = 0;
}

KeyEventScanner::Status KeyEventScanner::next(uint8_t byte)
{
    if (step_ == Step::kNone) {
        if (byte == 0x09) {
            result_ = Result::kTab;
            return Status::kEnsure;
        } else if (byte == 0x7f || byte == 0x08) {
            result_ = Result::kBackspace;
            return Status::kEnsure;
        } else if (byte == 0x0d) {
            step_ = Step::k0d;
            return Status::kUnsure;
        } else if (byte == 0x0a) {
            result_ = Result::kEnter;
            return Status::kEnsure;
        } else if (byte == 0x1b) {
            step_ = Step::k1b;
            return Status::kUnsure;
        } else if (byte == 0xc2) {
            step_ = Step::kc2;
            return Status::kUnsure;
        } else if (::isprint(byte)) {
            result_ = Result::kPrintable;
            extra_ = byte;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k0d) {
        if (byte == 0x00 || byte == 0x0a) {
            result_ = Result::kEnter;
            return Status::kEnsure;
        }
    } else if (step_ == Step::kc2) {
        if (::islower(byte - 0x20)) {
            result_ = Result::kCtrlAltPlus;
            extra_ = byte - 0x20;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b) {
        if (byte == 0x5b) {
            step_ = Step::k1b5b;
            return Status::kUnsure;
        } else if (byte == 0x4f) {
            step_ = Step::k1b4f;
            return Status::kUnsure;
        } else if (::isprint(byte)) {
            result_ = Result::kAltPlus;
            extra_ = byte;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b) {
        if (byte == 0x41) {
            result_ = Result::kMoveUp;
            return Status::kEnsure;
        } else if (byte == 0x42) {
            result_ = Result::kMoveDown;
            return Status::kEnsure;
        } else if (byte == 0x43) {
            result_ = Result::kMoveRight;
            return Status::kEnsure;
        } else if (byte == 0x44) {
            result_ = Result::kMoveLeft;
            return Status::kEnsure;
        } else if (byte == 0x31) {
            step_ = Step::k1b5b31;
            return Status::kUnsure;
        } else if (byte == 0x32) {
            step_ = Step::k1b5b32;
            return Status::kUnsure;
        } else if (byte == 0x33) {
            step_ = Step::k1b5b33;
            return Status::kUnsure;
        } else if (byte == 0x34) {
            step_ = Step::k1b5b34;
            return Status::kUnsure;
        } else if (byte == 0x35) {
            step_ = Step::k1b5b35;
            return Status::kUnsure;
        } else if (byte == 0x36) {
            step_ = Step::k1b5b36;
            return Status::kUnsure;
        }
    } else if (step_ == Step::k1b4f) {
        if (byte >= 0x50 && byte <= 0x53) {
            if (byte == 0x50)
                result_ = Result::kF1;
            else if (byte == 0x51)
                result_ = Result::kF2;
            else if (byte == 0x52)
                result_ = Result::kF3;
            else if (byte == 0x53)
                result_ = Result::kF4;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b31) {
        if (byte == 0x7e) {
            result_ = Result::kHome;
            return Status::kEnsure;
        } else if (byte == 0x35) {
            step_ = Step::k1b5b3135;
            return Status::kUnsure;
        } else if (byte == 0x37) {
            step_ = Step::k1b5b3137;
            return Status::kUnsure;
        } else if (byte == 0x38) {
            step_ = Step::k1b5b3138;
            return Status::kUnsure;
        } else if (byte == 0x39) {
            step_ = Step::k1b5b3139;
            return Status::kUnsure;
        }
    } else if (step_ == Step::k1b5b32) {
        if (byte == 0x7e) {
            result_ = Result::kInsert;
            return Status::kEnsure;
        } else if (byte == 0x30) {
            step_ = Step::k1b5b3230;
            return Status::kUnsure;
        } else if (byte == 0x31) {
            step_ = Step::k1b5b3231;
            return Status::kUnsure;
        } else if (byte == 0x33) {
            step_ = Step::k1b5b3233;
            return Status::kUnsure;
        } else if (byte == 0x34) {
            step_ = Step::k1b5b3234;
            return Status::kUnsure;
        }
    } else if (step_ == Step::k1b5b33) {
        if (byte == 0x7e) {
            result_ = Result::kDelete;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b34) {
        if (byte == 0x7e) {
            result_ = Result::kEnd;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b35) {
        if (byte == 0x7e) {
            result_ = Result::kPageUp;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b36) {
        if (byte == 0x7e) {
            result_ = Result::kPageDown;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b3135) {
        if (byte == 0x7e) {
            result_ = Result::kF5;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b3137) {
        if (byte == 0x7e) {
            result_ = Result::kF6;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b3138) {
        if (byte == 0x7e) {
            result_ = Result::kF7;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b3139) {
        if (byte == 0x7e) {
            result_ = Result::kF8;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b3230) {
        if (byte == 0x7e) {
            result_ = Result::kF9;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b3231) {
        if (byte == 0x7e) {
            result_ = Result::kF10;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b3233) {
        if (byte == 0x7e) {
            result_ = Result::kF11;
            return Status::kEnsure;
        }
    } else if (step_ == Step::k1b5b3234) {
        if (byte == 0x7e) {
            result_ = Result::kF12;
            return Status::kEnsure;
        }
    }

    step_ = Step::kNone;
    return Status::kFail;
}

KeyEventScanner::Status KeyEventScanner::stop()
{
    if (step_ == Step::k1b) {
        result_ = Result::kESC;
        return Status::kEnsure;
    } else if (step_ == Step::k0d) {
        result_ = Result::kEnter;
        return Status::kEnsure;
    } else {
        step_ = Step::kNone;
        return Status::kFail;
    }
}

}
}
