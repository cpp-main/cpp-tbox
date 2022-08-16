#include "session.h"
#include <tbox/base/log.h>

#include "connection.h"

namespace tbox {
namespace terminal {

Session::Session(Connection *wp_conn, const SessionToken &st) :
    wp_conn_(wp_conn),
    st_(st)
{ }

bool Session::send(char ch) const
{
    return wp_conn_->send(st_, ch);
}

bool Session::send(const std::string &str) const
{
    return wp_conn_->send(st_, str);
}

bool Session::endSession() const
{
    return wp_conn_->endSession(st_);
}

bool Session::isValid() const
{
    return wp_conn_->isValid(st_);
}

}
}
