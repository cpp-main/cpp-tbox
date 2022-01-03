#ifndef TBOX_UTIL_FS_H_20220103
#define TBOX_UTIL_FS_H_20220103

#include <string>

namespace tbox::util::fs {

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

/**
 * 将字串写入到文件
 */
bool WriteStringToTextFile(const std::string &filename, const std::string &content);

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
 */
bool WriteBinaryToFile(const std::string &filename, const std::string &content);

/**
 * 删除文件
 *
 * \param filename      被删除的文件名
 *
 * \return true 成功
 * \return false 失败
 */
bool RemoveFile(const std::string &filename);

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
 *
 * \return true     目录创建成功
 * \return false    目录创建失败
 */
bool MakeDirectory(const std::string &dir);

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

#endif //TBOX_UTIL_FS_H_20220103
