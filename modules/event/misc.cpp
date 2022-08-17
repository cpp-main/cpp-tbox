#include "misc.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <tbox/base/log.h>

namespace tbox {
namespace event {

bool CreateFdPair(int &read_fd, int &write_fd)
{
    int fds[2] = { 0 };
    if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) != 0) {  //!FIXME
        LogErr("pip2() fail, ret:%d", errno);
        return false;
    }

    read_fd = fds[0];
    write_fd = fds[1];
    return true;
}

}
}
