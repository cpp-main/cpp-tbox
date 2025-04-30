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
#include "fs.h"

#include <sys/stat.h>
#include <exception>
#include <fstream>
#include <errno.h>
#include <cstring>
#include <dirent.h>

#include <unistd.h>
#include <fcntl.h>

#include <tbox/base/log.h>
#include <tbox/base/scope_exit.hpp>

namespace tbox {
namespace util {
namespace fs {

using std::ifstream;
using std::ofstream;
using std::exception;

FileType GetFileType(const std::string &file_path)
{
    struct stat st;

    if (::stat(file_path.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode))    return FileType::kDirectory;
        if (S_ISREG(st.st_mode))    return FileType::kRegular;
        if (S_ISCHR(st.st_mode))    return FileType::kCharacterDevice;
        if (S_ISBLK(st.st_mode))    return FileType::kBlockDevice;
        if (S_ISLNK(st.st_mode))    return FileType::kSymbolLink;
        if (S_ISSOCK(st.st_mode))   return FileType::kSocket;
        if (S_ISFIFO(st.st_mode))   return FileType::kNamedPipe;
    }

    return FileType::kNone;
}

bool IsFileExist(const std::string &filename)
{
    int ret = ::access(filename.c_str(), F_OK);
    return ret == 0;
}

bool ReadStringFromTextFile(const std::string &filename, std::string &content)
{
    try {
        ifstream f(filename);
        if (f) {
            content = std::string((std::istreambuf_iterator<char>(f)),
                                   std::istreambuf_iterator<char>());
            return true;
        } else {
            LogWarn("open failed, %s", filename.c_str());
        }
    } catch (const exception &e) {
        LogWarn("catch exception: %s", e.what());
    }
    return false;
}

bool ReadEachLineFromTextFile(const std::string &filename, const std::function<void(const std::string&)> &line_handle_func)
{
    try {
        ifstream f(filename);
        if (f) {
            std::string text;
            while (std::getline(f, text)) {
                while (!text.empty() && (text.back() == '\r' || text.back() == '\n'))
                    text.pop_back();
                line_handle_func(text);
            }
            return true;
        } else {
            LogWarn("open failed, %s", filename.c_str());
        }
    } catch (const exception &e) {
        LogWarn("catch exception: %s", e.what());
    }
    return false;
}

bool ReadAllLinesFromTextFile(const std::string &filename, std::vector<std::string> &text_vec)
{
    try {
        ifstream f(filename);
        if (f) {
            std::string text;
            while (std::getline(f, text)) {
                while (!text.empty() && (text.back() == '\r' || text.back() == '\n'))
                    text.pop_back();
                text_vec.emplace_back(std::move(text));
            }
            return true;
        } else {
            LogWarn("open failed, %s", filename.c_str());
        }
    } catch (const exception &e) {
        LogWarn("catch exception: %s", e.what());
    }
    return false;
}

bool ReadFirstLineFromTextFile(const std::string &filename, std::string &text)
{
    try {
        ifstream f(filename);
        if (f) {
            std::getline(f, text);
            while (!text.empty() && (text.back() == '\r' || text.back() == '\n'))
                text.pop_back();
            return true;
        } else {
            LogWarn("open failed, %s", filename.c_str());
        }
    } catch (const exception &e) {
        LogWarn("catch exception: %s", e.what());
    }
    return false;
}

bool WriteStringToTextFile(const std::string &filename, const std::string &content, bool sync_now)
{
    return WriteFile(filename.c_str(), content.data(), content.size(), sync_now);
}

bool AppendStringToTextFile(const std::string &filename, const std::string &content, bool sync_now)
{
    return AppendFile(filename.c_str(), content.data(), content.size(), sync_now);
}

bool ReadBinaryFromFile(const std::string &filename, std::string &content)
{
    return ReadStringFromTextFile(filename, content);
}

bool WriteBinaryToFile(const std::string &filename, const std::string &content, bool sync_now)
{
    return WriteFile(filename.c_str(), content.data(), content.size(), sync_now);
}

bool WriteFile(const char *filename, const void *data_ptr, size_t data_size, bool sync_now)
{
    //! 这里采用原始系统接口，是为了解决Linux下往文件写数据不会立即同步到外存的问题。
    //! 这个问题会在导致写入数据之后，系统宕机或设备断电，重启后文件中数据丢失的故障。
    //! 为保证数据一定写入到外存，需要在写入数据之后，调用一次fsync(fd)。
    //! 然而，标准C++的ofstream无法提供文件对象的fd。所以不得不使用原始的文件操作。

    int flag = O_CREAT | O_WRONLY | O_TRUNC;
    if (sync_now)
        flag |= O_SYNC; //! 加了这个参数后，相当于每次write()操作都执行了一次fsync(fd)

    int fd = ::open(filename, flag, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        SetScopeExitAction([fd] { close(fd); });  //! 确保退出时一定会close(fd)

        auto wsize = ::write(fd, data_ptr, data_size);
        if (wsize == static_cast<ssize_t>(data_size))
            return true;

        if (wsize == -1)
            LogWarn("write errno:%d, %s", errno, strerror(errno));
        else
            LogWarn("wsize:%d, size:%d", wsize, data_size);

    } else
        LogWarn("open %s failed, %d, %s", filename, errno, strerror(errno));

    return false;
}

bool AppendFile(const char *filename, const void *data_ptr, size_t data_size, bool sync_now)
{
    int flag = O_CREAT | O_WRONLY | O_APPEND;
    if (sync_now)
        flag |= O_SYNC;

    int fd = ::open(filename, flag, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        SetScopeExitAction([fd] { close(fd); });  //! 确保退出时一定会close(fd)

        auto wsize = ::write(fd, data_ptr, data_size);
        if (wsize == static_cast<ssize_t>(data_size))
            return true;

        if (wsize == -1)
            LogWarn("write errno:%d, %s", errno, strerror(errno));
        else
            LogWarn("wsize:%d, size:%d", wsize, data_size);

    } else
        LogWarn("open %s failed, %d, %s", filename, errno, strerror(errno));

    return false;
}

bool RemoveFile(const std::string &filename, bool allow_log_print)
{
    int ret = ::unlink(filename.c_str());
    if (ret == 0)
        return true;

    if (errno != ENOENT && allow_log_print)
        LogWarn("errno:%d (%s)", errno, strerror(errno));

    return false;
}

bool MakeSymbolLink(const std::string &old_path, const std::string &new_path, bool allow_log_print)
{
    int ret = ::symlink(old_path.c_str(), new_path.c_str());
    if (ret == 0)
        return true;

    if (allow_log_print)
        LogWarn("errno:%d (%s)", errno, strerror(errno));
    return false;
}

bool MakeLink(const std::string &old_path, const std::string &new_path, bool allow_log_print)
{
    int ret = ::link(old_path.c_str(), new_path.c_str());
    if (ret == 0)
        return true;

    if (allow_log_print)
        LogWarn("errno:%d (%s)", errno, strerror(errno));
    return false;
}

bool IsDirectoryExist(const std::string &dir)
{
    return GetFileType(dir) == FileType::kDirectory;
}

bool MakeDirectory(const std::string &origin_dir_path, bool allow_log_print)
{
    if (origin_dir_path.empty()) {
        if (allow_log_print)
            LogWarn("origin_dir_path is empty");
        return false;
    }

    if (IsDirectoryExist(origin_dir_path))
        return true;

    std::string trimmed_dir_path;
    trimmed_dir_path.reserve(origin_dir_path.size());
    for (auto c : origin_dir_path) {
        //! 消除重复的 '/'，比如: "a//b"
        if (trimmed_dir_path.empty() || !(c == '/' && trimmed_dir_path.back() == '/'))
            trimmed_dir_path.push_back(c);
    }

    //! 确保trimmed_dir_path以'/'字符结尾
    if (trimmed_dir_path.back() != '/')
        trimmed_dir_path.push_back('/');

    for (size_t i = 1; i < trimmed_dir_path.size(); ++i) {
        if (trimmed_dir_path.at(i) == '/') {
            trimmed_dir_path.at(i) = '\0';
            struct stat sb;
            if (::stat(trimmed_dir_path.c_str(), &sb) != 0) {
                if (errno == ENOENT) {  //! 如果trimmed_dir_path指定的inode不存在，则创建目录
                    if (::mkdir(trimmed_dir_path.c_str(), 0775) != 0) {
                        if (allow_log_print)
                            LogWarn("mkdir(%s) fail, errno:%d, %s", trimmed_dir_path.c_str(), errno, strerror(errno));
                        return false;
                    }
                } else {    //! 如果是其它的错误
                    if (allow_log_print)
                        LogWarn("stat(%s) fail, errno:%d, %s", trimmed_dir_path.c_str(), errno, strerror(errno));
                    return false;
                }
            } else {    //! 如果 trimmed_dir_path 指定的inode存在
                if (!S_ISDIR(sb.st_mode)) {  //! 该inode并不是一个目录
                    if (allow_log_print)
                        LogWarn("inode %s is not directory", trimmed_dir_path.c_str());
                    return false;
                }
                //! 存在，且是目录，则不做任何事件
            }
            trimmed_dir_path.at(i) = '/';
        }
    }

    return true;
}

bool RemoveDirectory(const std::string &dir, bool is_remove_file_only)
{
    DIR *dp = ::opendir(dir.c_str());
    if (dp == nullptr) {
        // 无法打开目录，直接结束
        if (errno == ENOENT) {
            LogWarn("directory %s does not exist", dir.c_str());
        } else {
            LogWarn("open directory %s fail, errno:%d, %s", dir.c_str(), errno, strerror(errno));
        }
        return false;
    }

    SetScopeExitAction([dp] { closedir(dp); });

    bool is_all_removed = true;
    struct dirent *entry = nullptr;

    while ((entry = readdir(dp)) != nullptr) {
        std::string entry_name = entry->d_name;
        if (entry_name == "." || entry_name == "..") {
            // 目录系统的 . 和 .. 内容，直接略过
            continue;
        }

        // 依次查看每一个文件或者目录的属性
        std::string full_path = dir + '/' + entry_name;
        struct stat statbuf;
        if (::stat(full_path.c_str(), &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                // 属性为目录，递归删除目录的内容
                if (!RemoveDirectory(full_path, is_remove_file_only)) {
                    is_all_removed = false;
                }
            } else {
                // 属性为文件，直接删除文件
                if (::remove(full_path.c_str())) {
                    LogWarn("removing file %s fail, errno:%d, %s", full_path.c_str(), errno, strerror(errno));
                    is_all_removed = false;
                }
            }
        } else {
            // 无法获取属性，直接结束
            LogWarn("getting state of %s fail, errno:%d, %s", full_path.c_str(), errno, strerror(errno));
            is_all_removed = false;
        }
    }

    if (!is_remove_file_only) {
       // 最后删除目录
       if (::rmdir(dir.c_str())) {
            LogWarn("removing directory %s fail, errno:%d, %s", dir.c_str(), errno, strerror(errno));
            is_all_removed = false;
        }
    }

    return is_all_removed;
}

bool ListDirectory(const std::string &dir, std::vector<std::string> &names)
{
    DIR *dp = ::opendir(dir.c_str());
    if (dp == nullptr) {
        // 无法打开目录，直接结束
        if (errno == ENOENT) {
            LogWarn("directory %s does not exist", dir.c_str());
        } else {
            LogWarn("open directory %s fail, errno:%d, %s", dir.c_str(), errno, strerror(errno));
        }
        return false;
    }

    SetScopeExitAction([dp] { closedir(dp); });

    struct dirent *entry = nullptr;
    while ((entry = readdir(dp)) != nullptr) {
        std::string entry_name = entry->d_name;
        if (entry_name != "." && entry_name != "..") {
            names.emplace_back(entry_name);
        }
    }

    return true;
}

std::string Basename(const std::string &full_path)
{
    auto const pos = full_path.find_last_of('/');
    return full_path.substr(pos + 1);
}

const char* Basename(const char *full_path)
{
    const char *p_last = full_path;
    if (p_last != nullptr) {
        for (const char *p = full_path; *p; ++p) {
            if (*p == '/')
                p_last = p + 1;
        }
    }
    return p_last;
}

/**
 * 目标:
 * "a" -> "."
 * "a/" -> "a"
 * "a/b" -> "a"
 * "a/b/" -> "a/b"
 * " a/b " -> "a"
 * "" -> "."
 * "abc" -> "."
 * "/" -> "/"
 * "/a" -> "/"
 * "/a/" -> "/a"
 */
std::string Dirname(const std::string &full_path)
{
    auto start_pos = full_path.find_first_not_of("\t ");    //! 去除左边的空白符
    auto end_pos = full_path.find_last_of('/');

    //! 如果全是空白符或没有找到/，则推测为当前目录
    if (start_pos == std::string::npos || end_pos == std::string::npos)
        return ".";

    //! 对以/开头的要特征处理，否则 "/a" 就会被处理成 ""
    if (start_pos == end_pos)
        return "/";

    return full_path.substr(start_pos, end_pos - start_pos);
}

bool Rename(const std::string &old_name, const std::string &new_name)
{
    int ret = ::rename(old_name.c_str(), new_name.c_str());
    if (ret == 0)
        return true;

    LogWarn("rename '%s' to '%s' fail, errno:%d (%s)",
            old_name.c_str(), new_name.c_str(), errno, strerror(errno));
    return false;
}

}
}
}
