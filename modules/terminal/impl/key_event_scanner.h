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
#ifndef TBOX_TERMINAL_KEYEVENT_SCANNER_H_20220203
#define TBOX_TERMINAL_KEYEVENT_SCANNER_H_20220203

#include <cstdint>

namespace tbox {
namespace terminal {

/**
 * 关键词扫描器
 *
 * 根据输入的字符序列，提取出用户的按键动作
 *
 * 按键动键与对应的字符序列：
 * - Tab:       09
 * - Backspace: 7f|08
 * - ESC:       1b
 * - Enter:     0d [00|0a]
 *              0a
 *
 * - Alt+?:     1b ?
 * - Ctrl+Alt+?: c2 ?+0x20
 *
 * - MoveUp:    1b 5b 41
 * - MoveDown:  1b 5b 42
 * - MoveRight: 1b 5b 43
 * - MoveLeft:  1b 5b 44
 *
 * - Home:      1b 5b 31 7e
 * - Insert:    1b 5b 32 7e
 * - Delete:    1b 5b 33 7e
 * - End:       1b 5b 34 7e
 * - PageUp:    1b 5b 35 7e
 * - PageDown:  1b 5b 36 7e
 *
 * - F1:        1b 4f 50
 * - F2:        1b 4f 51
 * - F3:        1b 4f 52
 * - F4:        1b 4f 53
 * - F5:        1b 5b 31 35 7e
 * - F6:        1b 5b 31 37 7e
 * - F7:        1b 5b 31 38 7e
 * - F8:        1b 5b 31 39 7e
 * - F9:        1b 5b 32 30 7e
 * - F10:       1b 5b 31 31 7e
 * - F11:       1b 5b 31 33 7e
 * - F12:       1b 5b 31 34 7e
 *
 * 本扫描器使用状态机进行识别。
 * 在开始识别时，先调用 start() 重置状态。
 * 然后逐一将收到的字符通过调用 next() 喂给扫描器，判断其返回值，见 Status 定义。
 * 如果处理完最后一个字符都是 kUnsure 状态，在扫描完最后一个字符后要调用 stop()，再判断其返回值。
 * 如果返回了 kEnsure，则可以从 result() 获取识别的结果。如果是组合键，可进一步通过 extra() 获取组合键的值。
 */
class KeyEventScanner {
  public:
    enum class Status {
        kUnsure,    //!< 尚不确定
        kEnsure,    //!< 已确定
        kFail,      //!< 匹配失败
    };

    void start();
    Status next(uint8_t byte);
    Status stop();

    enum class Result {
        kNone,
        kPrintable,
        kTab, kBackspace, kESC, kEnter, //! 常规键
        kAltPlus, kCtrlAltPlus,         //!< 组合键
        kMoveUp, kMoveDown, kMoveLeft, kMoveRight,  //!< 方向键
        kHome, kInsert, kDelete, kEnd, kPageUp, kPageDown,
        kF1, kF2, kF3, kF4, kF5, kF6, kF7, kF8, kF9, kF10, kF11, kF12   //!< Fn 键
    };

    inline Result result() const { return result_; }
    inline uint8_t extra() const { return extra_; }

  private:
    enum class Step {
        kNone,
        k0d, kc2,
        k1b, k1b5b,
        k1b5b31, k1b5b3135, k1b5b3137, k1b5b3138, k1b5b3139,
        k1b5b32, k1b5b3230, k1b5b3231, k1b5b3233, k1b5b3234,
        k1b5b33,
        k1b5b34,
        k1b5b35,
        k1b5b36,
        k1b4f,
    };

    Result result_ = Result::kNone;
    uint8_t extra_ = 0;
    Step step_ = Step::kNone;
};

}
}

#endif //TBOX_TERMINAL_KEYEVENT_SCANNER_H_20220203
