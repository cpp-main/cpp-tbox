#ifndef TBOX_TERMINAL_SESSION_IMP_H_20220204
#define TBOX_TERMINAL_SESSION_IMP_H_20220204

#include "key_event_scanner.h"
#include <deque>

namespace tbox::terminal {

class SessionImpl : public Session {
  public:
    SessionImpl(Connection *wp_conn) :
        wp_conn_(wp_conn)
    { }

    ~SessionImpl() { }

    void setSessionToken(const SessionToken &token)
    { token_ = token; }

    void setWindowSize(uint16_t w, uint16_t h)
    { window_width_ = w; window_height_ = h; }

  public:
    bool send(char ch) override { return wp_conn_->send(token_, ch); }
    bool send(const std::string &str) override { return wp_conn_->send(token_, str); }
    void endSession() override { wp_conn_->endSession(token_); }
    bool isValid() const override { return wp_conn_->isValid(token_); }

    bool window_width() const override { return window_width_; }
    bool window_height() const override { return window_height_; }

  public:
    std::string curr_input;
    size_t cursor = 0;

    Path path;  //! 当前路径
    std::deque<std::string> history;   //! 历史命令
    size_t history_index = 0;   //! 0表示不指定历史命令

    KeyEventScanner key_event_scanner_;

  private:
    Connection *wp_conn_ = nullptr;
    SessionToken token_;

    uint16_t window_width_ = 0;
    uint16_t window_height_ = 0;
};

}

#endif //TBOX_TERMINAL_SESSION_IMP_H_20220204
