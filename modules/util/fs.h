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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_UTIL_FS_H_20220103
#define TBOX_UTIL_FS_H_20220103

#include <string>
#include <vector>
#include <functional>

namespace tbox {
namespace util {
namespace fs {

////////////////////////////////////////////////////////////////////
// 文件相关
////////////////////////////////////////////////////////////////////

//! 文件类型
enum class FileType {
    kNone,              //!< 未知
    kDirectory,         //!< 目录
    kRegular,           //!< 常规文件
    kCharacterDevice,   //!< 字符设备
    kBlockDevice,       //!< 块设备
    kSymbolLink,        //!< 符号链接
    kSocket,            //!< 套接字
    kNamedPipe,         //!< 有名管道
};

/**
 * 获取文件类型
 *
 * \param file_path  文件路径
 * \return FileType  文件类型
 */
FileType GetFileType(const std::string &file_path);

/**
 * 检查文件是否存在
 *
 * \param filename      文件名
 *
 * \return true     文件存在
 * \return false    文件不存在
 */
bool IsFileExist(const std::string &filename);

/**
 * 从文件中读取字串
 *
 * \param filename      文件名
 * \param content       读取的文本输出的std::string
 *
 * \return true     成功
 * \return false    失败
 */
bool ReadStringFromTextFile(const std::string &filename, std::string &content);

/// 遍历文本中的每一行
/**
 * \param filename          文件名
 * \param line_handle_func  行字串处理函数
 *
 * \return true     文件打开成功
 * \return false    文件打开失败
 */
bool ReadEachLineFromTextFile(const std::string &filename, const std::function<void(const std::string&)> &line_handle_func);

/**
 * 一次性读取文件中所有行
 *
 * \param filename          文件名
 * \param lines             行文本数组
 *
 * \return true     文件打开成功
 * \return false    文件打开失败
 */
bool ReadAllLinesFromTextFile(const std::string &filename, std::vector<std::string> &lines);

/**
 * 从文件中读取第一行文本
 *
 * \param filename          文件名
 * \param text              读出的行内容
 *
 * \return true     文件打开成功
 * \return false    文件打开失败
 */
bool ReadFirstLineFromTextFile(const std::string &filename, std::string &text);

/**
 * 将字串写入到文件
 *
 * \param filename      文件名
 * \param content       将要写入的字串内容
 * \param sync_now      是否需要立即sync
 *
 * \return true     成功
 * \return false    失败
 */
bool WriteStringToTextFile(const std::string &filename, const std::string &content, bool sync_now = false);

/**
 * 将字串追加到文件尾部
 *
 * \param filename      文件名
 * \param content       将要写入的字串内容
 * \param sync_now      是否需要立即sync
 *
 * \return true     成功
 * \return false    失败
 */
bool AppendStringToTextFile(const std::string &filename, const std::string &content, bool sync_now = false);

/**
 * 从文件中读取数据
 *
 * \param filename      文件名
 * \param content       读取的数据，可能非字串
 *
 * \return true     成功
 * \return false    失败
 */
bool ReadBinaryFromFile(const std::string &filename, std::string &content);

/**
 * 将数据写入到文件
 *
 * \param filename      文件名
 * \param content       将要写入的数据内容
 * \param sync_now      是否需要立即sync
 *
 * \return true     成功
 * \return false    失败
 */
bool WriteBinaryToFile(const std::string &filename, const std::string &content, bool sync_now = false);

/**
 * 将数据写入到文件
 *
 * \param filename      文件名
 * \param data_ptr      将要写入的数据地址
 * \param data_size     将要写入的数据长度
 * \param sync_now      是否需要立即sync
 *
 * \return true     成功
 * \return false    失败
 */
bool WriteFile(const char *filename, const void *data_ptr, size_t data_size, bool sync_now = false);

/**
 * 将数据追加到文件尾部
 *
 * \param filename      文件名
 * \param data_ptr      将要写入的数据地址
 * \param data_size     将要写入的数据长度
 * \param sync_now      是否需要立即sync
 *
 * \return true     成功
 * \return false    失败
 */
bool AppendFile(const char *filename, const void *data_ptr, size_t data_size, bool sync_now = false);

/**
 * 删除文件
 *
 * \param filename      被删除的文件名
 *
 * \return true 成功
 * \return false 失败
 */
bool RemoveFile(const std::string &filename, bool allow_log_print = true);

/// 创建符号链接文件
/**
 * \param old_path      源路径
 * \param new_path      符号链接文件的路径
 * \param allow_log_print   是否允许错误日志打印
 *
 * \return true 成功
 * \return false 失败
 */
bool MakeSymbolLink(const std::string &old_path, const std::string &new_path, bool allow_log_print = true);

/// 创建链接文件
/**
 * \param old_path      源路径
 * \param new_path      符号链接文件的路径
 * \param allow_log_print   是否允许错误日志打印
 *
 * \return true 成功
 * \return false 失败
 */
bool MakeLink(const std::string &old_path, const std::string &new_path, bool allow_log_print = true);

////////////////////////////////////////////////////////////////////
// 目录相关
////////////////////////////////////////////////////////////////////

/**
 * 目录是否存在
 *
 * \param dir               目录
 *
 * \return true     目录存在
 * \return false    目录不存在
 */
bool IsDirectoryExist(const std::string &dir);

/**
 * 创建目录
 * 等价于shell命令 "mkdir -p xxx"
 *
 * \param dir               目录
 * \param allow_log_print   是否允许错误日志打印
 *                          在日志模块中使用要禁用，其它默认打开即可
 *
 * \return true     目录创建成功
 * \return false    目录创建失败
 */
bool MakeDirectory(const std::string &dir, bool allow_log_print = true);

/**
 * 递归删除指定目录
 * 等价于shell命令："rm -rf xxx"
 *
 * \param dir                   需要删除的目录路径，路径需要全路径，如 /data/test
 * \param is_remove_file_only   保存目录结构，仅删除文件
 *
 * \return true     目录被完全删除
 * \return false    目录未被完全删除
*/
bool RemoveDirectory(const std::string &dir, bool is_remove_file_only = false);

/**
 * 列出指定目录下的子文件与目录
 * 等价于shell命令："ls"
 *
 * \param dir       需要删除的目录路径，路径需要全路径，如 /data/test
 *
 * \return true     成功
 * \return false    失改
*/
bool ListDirectory(const std::string &dir, std::vector<std::string> &names);

////////////////////////////////////////////////////////////////////
// 其它
////////////////////////////////////////////////////////////////////

/**
 * 根据路径获取文件名
 */
std::string Basename(const std::string &full_path);
const char* Basename(const char *full_path);

/**
 * 根据路径获取目录名
 */
std::string Dirname(const std::string &full_path);

/**
 * 重命名
 *
 * \param old_name  旧文件名
 * \param new_name  新文件名
 *
 * \return true 成功
 * \return false 失败
 */
bool Rename(const std::string &old_name, const std::string &new_name);

}
}
}

#endif //TBOX_UTIL_FS_H_20220103
