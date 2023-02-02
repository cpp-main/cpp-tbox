#include "fs.h"

#include <sys/stat.h>
#include <exception>
#include <fstream>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <tbox/base/log.h>

namespace tbox {
namespace util {
namespace fs {

using std::ifstream;
using std::ofstream;
using std::exception;

bool IsFileExist(const std::string &filename)
{
#if 1
    int ret = ::access(filename.c_str(), F_OK);
    return ret == 0;
#else
    ifstream ifs(filename);
    return ifs.is_open();
#endif
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

bool WriteStringToTextFile(const std::string &filename, const std::string &content)
{
    ofstream ofs;
    try {
        ofs.open(filename, ofstream::out | ofstream::trunc);
        if (ofs.is_open()) {
            ofs << content;
            return true;
        } else {
            LogWarn("open failed, %s", filename.c_str());
        }
    } catch (const exception &e) {
        LogWarn("catch exception: %s", e.what());
    }
    return false;
}

bool ReadBinaryFromFile(const std::string &filename, std::string &content)
{
    return ReadStringFromTextFile(filename, content);
}

bool WriteBinaryToFile(const std::string &filename, const std::string &content)
{
    return WriteStringToTextFile(filename, content);
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
    struct stat sb;
    //! 如果读到dir的inode信息，且该inode是DIR，则返回true
    return ((::stat(dir.c_str(), &sb) == 0) && S_ISDIR(sb.st_mode));
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

bool RemoveDirectory(const std::string &dir)
{
    LogUndo();
    return false;
}

std::string Basename(const std::string &full_path)
{
    auto const pos = full_path.find_last_of('/');
    return full_path.substr(pos + 1);
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

}
}
}
