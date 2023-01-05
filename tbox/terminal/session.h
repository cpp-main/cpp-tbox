#ifndef TBOX_TERMINAL_SESSION_H_20220214
#define TBOX_TERMINAL_SESSION_H_20220214

#include "types.h"

namespace tbox {
namespace terminal {

class Connection;

class Session {
  public:
    Session(Connection *wp_conn, const SessionToken &st);

    bool send(char ch) const;
    bool send(const std::string &str) const;
    bool endSession() const;
    bool isValid() const;

  private:
    Connection *wp_conn_;
    const SessionToken st_;
};

}
}

#endif //TBOX_TERMINAL_SESSION_H_20220214
