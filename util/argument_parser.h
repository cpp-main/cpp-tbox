#ifndef TBOX_UTIL_ARGUMENT_PARSER_H_20220105
#define TBOX_UTIL_ARGUMENT_PARSER_H_20220105

#include <functional>

namespace tbox::util {

class ArgumentParser {
  public:
    class OptStr {
        friend ArgumentParser;

      public:
        bool isNull() const { return str_.empty(); }
        bool isUsed() const { return used_; }

        std::string str() {
            used_ = true;
            return str_;
        }

      private:
        void set(const std::string &str) { str_ = str; }

        bool used_ = false;
        std::string str_;
    };

    using Handler = std::function<bool(char /*short_opt*/,const std::string& /*long_opt*/, OptStr&)>;

    ArgumentParser(const Handler &handler) : handler_(handler) { }

    bool parse(int argc, const char * const * const argv);

  private:
    Handler handler_;
};

}

#endif //TBOX_UTIL_ARGUMENT_PARSER_H_20220105
