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
/**
 * This file achived an arguemnt parser.
 *
 * Example:
 *
 *   test -h -l 6
 *   test --help --level 6
 *   test --help --level=6
 *
 * Code:
 *
 * int main(int argc, char **argv)
 * {
 *     bool print_help = false;
 *     int  level = 0;
 *     tbox::util::ArgumentParser parser(
 *         [&](char short_opt, const std::string &long_opt,
 *             ArgumentParser::OptionValue& opt_value) {
 *             if (short_opt == 'h' || long_opt == "help") {
 *                 print_help = true;
 *             } else if (short_opt == 'l' || long_opt == "level") {
 *                 level = std::stoi(opt_value.get());
 *             } else {
 *                 if (short_opt != 0)
 *                     cerr << "Error: invalid option `-" << short_opt << "'" << endl;
 *                 else
 *                     cerr << "Error: invalid option `--" << long_option << "'" << endl;
 *                 return false;
 *             }
 *             return true;
 *         }
 *     );
 *
 *     if (!parser(argc, argv))
 *         return 0;
 *
 *     //! ... you app code ...
 * }
 */
#ifndef TBOX_UTIL_ARGUMENT_PARSER_H_20220105
#define TBOX_UTIL_ARGUMENT_PARSER_H_20220105

#include <string>
#include <vector>
#include <functional>

namespace tbox {
namespace util {

class ArgumentParser {
  public:
    class OptionValue {
        friend ArgumentParser;

      public:
        //! 判断是否有效
        inline bool valid () const { return valid_; }

        //! 获取值，并标记使用
        std::string get() {
            if (valid_)
                used_ = true;
            return str_;
        }

      private:
        inline void set(const std::string &str) {
            str_ = str;
            valid_ = true;
        }

        inline bool isUsed() const { return used_; }

      private:
        bool valid_ = false;
        bool used_  = false; //!< 是否已被 get()
        std::string str_;
    };

    /**
     * \brief   解析处理回调函数
     *
     * \param   short_option    短参数项，如: -h, -v, -pnl 等
     * \param   long_option     长参数项，如: --help, --version 等
     * \param   opt_value       参数值
     *
     * \return  true    继续解析
     * \return  false   停止解析
     *
     * \note
     *   关于 short_option 与 long_option
     *   - 当解析到短参数项时，short_option 为参数项的值，而 long_option 字串为空。
     *   - 当解析到长参数项时，short_option 为 0，而 long_option 字串为参数项值。
     *
     *   关于 opt_value
     *   - 当该参数项有可用的参数值时，opt_value.isNull() 返回 false，opt_value.get() 可获取到该字串。
     *     如：'-s abc -n'，当解析到 s 参数时，opt_value 有值，且为 'abc'。而解释到参数项 'n' 时，
     *     opt_value 则无值。
     *   - 如果在 hanele 中调用了 opt_value.get()，解析器将不会将 opt_value 对应的字串当成参数项
     */
    using Handler = std::function<
        bool (char,     //!< short_option
              const std::string&,   //!< long_option
              OptionValue&) //!< opt_value
    >;

    ArgumentParser(const Handler &handler) : handler_(handler) { }

    /**
     * \brief   解析
     *
     * \param   argc    参数个数
     * \param   argv    参数列表
     * \param   start   参数起始
     *
     * \return  true    解析完成
     * \return  false   解析失败
     */
    bool parse(int argc, const char * const * const argv, int start = 1);
    bool parse(const std::vector<std::string> &args, int start = 1);

  private:
    Handler handler_;
};

}
}

#endif //TBOX_UTIL_ARGUMENT_PARSER_H_20220105
