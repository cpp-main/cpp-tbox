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
#ifndef TBOX_UTIL_FS_H_20220103
#define TBOX_UTIL_FS_H_20220103

#include <string>
#include <functional>

namespace tbox {
namespace util {
namespace fs {

////////////////////////////////////////////////////////////////////
// 文件相关
////////////////////////////////////////////////////////////////////

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

/// 便利文本中的每一行
/**
 * \param filename          文件名
 * \param line_handle_func  行字串处理函数
 *
 * \return true     文件打开成功
 * \return false    文件打开失败
 */
bool ReadEachLineFromTextFile(const std::string &filename, const std::function<void(const std::string&)> &line_handle_func);

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
 * 删除目录
 *
 * \param dir               目录
 *
 * \return true     目录删除成功
 * \return false    目录删除失败
 */
bool RemoveDirectory(const std::string &dir);

////////////////////////////////////////////////////////////////////
// 其它
////////////////////////////////////////////////////////////////////

/**
 * 根据路径获取文件名
 */
std::string Basename(const std::string &full_path);

/**
 * 根据路径获取目录名
 */
std::string Dirname(const std::string &full_path);

}
}
}

#endif //TBOX_UTIL_FS_H_20220103
