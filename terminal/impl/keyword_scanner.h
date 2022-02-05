#ifndef TBOX_TERMINAL_KEYWORD_SCANNER_H_20220203
#define TBOX_TERMINAL_KEYWORD_SCANNER_H_20220203

#include <cstdint>
#include <vector>

namespace tbox::terminal {

class KeywordScanner {
  public:
    enum class Status {
        kUnsure,    //!< 还不确定
        kEnsure,    //!< 已确定
        kFail,      //!< 未能匹配
    };

    void start();
    Status next(uint8_t byte);
    Status stop();

    enum class Result {
        kNone,
        kPrintable,
        kTab, kBackspace, kESC, kEnter,
        kAltPlus, kCtrlAltPlus,
        kMoveUp, kMoveDown, kMoveLeft, kMoveRight,
        kHome, kInsert, kDelete, kEnd, kPageUp, kPageDown,
        kF1, kF2, kF3, kF4,
        kF5, kF6, kF7, kF8,
        kF9, kF10, kF11, kF12
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
        k1b5b33, k1b5b34, k1b5b35, k1b5b36,
        k1b4f,
    };

    Result result_ = Result::kNone;
    uint8_t extra_ = 0;
    Step step_ = Step::kNone;
};

}

#endif //TBOX_TERMINAL_KEYWORD_SCANNER_H_20220203
