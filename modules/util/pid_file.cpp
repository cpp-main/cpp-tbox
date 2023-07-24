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
#include "pid_file.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <cstring>

#include <iostream>
#include <tbox/base/scope_exit.hpp>

using namespace std;

namespace tbox {
namespace util {

PidFile::PidFile()
{ }

PidFile::~PidFile()
{
    unlock();
}

bool PidFile::lock(const std::string &pid_filename)
{
    if (fd_ >= 0)   //! 已经 initialize() 过，直接返回
        return true;

    pid_filename_ = pid_filename;

    int fd = open(pid_filename_.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC, 0644);
    if (fd < 0) {
        cerr << "Err: create pid file " << pid_filename_ << " fail. errno:" << errno << ", " << strerror(errno) <<  endl;
        return false;
    }

    //! 注册函数退出时 close(fd) 动作
    ScopeExitActionGuard close_fd([fd] { close(fd); });

    //! 加锁
    struct flock flock = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };

    int ret = fcntl(fd, F_SETLK, &flock);
    if (ret < 0) {
        if (errno == EAGAIN)  //! 已被其它进程锁住了
            return false;

        cerr << "Err: lock pid file fail, errno:" << errno << ", " << strerror(errno) << endl;
        return false;
    }

    //! 加锁成功，写入 pid
    pid_t pid = getpid();
    std::string pid_str = std::to_string(pid);
    ssize_t wsize = write(fd, pid_str.c_str(), pid_str.size());
    if (wsize < 0)
        cerr << "Warn: write pid fail. errno:" << errno << ", " << strerror(errno) << endl;

    close_fd.cancel();   //! 不要 close(fd);
    fd_ = fd;

    return true;
}

bool PidFile::unlock()
{
    if (fd_ < 0)
        return false;

    CHECK_CLOSE_RESET_FD(fd_);

    int ret = ::unlink(pid_filename_.c_str());
    if (ret < 0)
        cerr << "Warn: unlink pid file " << pid_filename_ << " fail. errno:" << errno << ", " << strerror(errno) << endl;

    return true;
}

}
}
