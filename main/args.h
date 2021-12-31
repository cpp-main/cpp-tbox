/**
 * 本文件定义 Args 类
 *
 * Args 类用于解释程序的命令参数，输出程序运行的配置Json数据。
 *
 * 参数：
 *
 * -s 'key=value' 参数，表示强制修改配置中的某个参数
 * -p 参数，表示执行前要打印配置Json字串
 * -n 参数，告知不需要运行
 *
 * 示例：
 *
 * myproc   # 采用默认参数运行
 * myproc -s 'log.level=6'  # 采用默认参数，并将log.level的值改成6
 * myproc -c somewhere/my_conf.json # 采用指定配置my_conf.json中的配置运行
 * myproc -c somewhere/my_conf.json -s 'log.level=6'    # 采用指定配置my_conf.json中的配置运行，并将log.level的值改成6
 *
 * myproc -h
 * myproc --help
 *
 * myproc -v
 * myproc --version
 */
#ifndef TBOX_MAIN_ARGS_H_20211229
#define TBOX_MAIN_ARGS_H_20211229

#include <tbox/base/json_fwd.h>

namespace tbox::main {

class Args {
  public:
    Args(Json &conf);

    /**
     * 解析参数
     *
     * \param   argc    参数个数
     * \param   argv    参数列表
     *
     * \return true     运行
     * \return false    停止运行
     */
    bool parse(int argc, char **argv);

  protected:
    void printHelp(const char *proc_name);
    void printVersion();

    void loadConfig(const char *config_filename);
    void set(const char *set_string);

  private:
    Json &conf_;
};

}

#endif //TBOX_MAIN_ARGS_H_20211229
