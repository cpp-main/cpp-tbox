#include "argument_parser.h"

#include <cassert>

namespace tbox::util {

namespace {
/**
 * \brief   pick key and value from '--key=value'
 */
bool PickKeyAndValue(const std::string &orig, std::string &key, std::string &value)
{
    auto pos = orig.find_first_of('=');
    if (pos == std::string::npos)
        return false;

    key   = orig.substr(0, pos);
    value = orig.substr(pos + 1);
    return true;
}
}

bool ArgumentParser::parse(int argc, const char * const * const argv, int start)
{
    assert(start >= 0);
    assert(argc > start);
    assert(argv != nullptr);

    for (int i = start; i < argc; ++i) {
        const char *curr_arg = argv[i];
        if (curr_arg == nullptr)
            return false;

        const std::string curr(curr_arg);
        OptionValue opt_value;
        if (i != (argc - 1)) {
            const char *next_arg = argv[i + 1];
            if (next_arg == nullptr)
                return false;
            opt_value.set(next_arg);
        }

        if (curr[0] == '-') {
            if (curr[1] == '-') {   //! 匹配 --xxxx
                const std::string opt = curr.substr(2);
                std::string key, value;
                //! handle '--key=value'
                if (PickKeyAndValue(opt, key, value)) {
                    OptionValue tmp;
                    tmp.set(value);
                    if (!handler_(0, key, tmp))
                        return false;
                } else {
                    //! handle '--key value'
                    if (!handler_(0, opt, opt_value))
                        return false;
                }
            } else {    //! handle -xyz pattern
                for (size_t j = 1; j < curr.size(); ++j) {
                    char opt = curr[j];
                    if (j != (curr.size() - 1)) {
                        OptionValue tmp;
                        if (!handler_(opt, "", tmp))
                            return false;
                    } else {
                        //! only this last opt possess value
                        if (!handler_(opt, "", opt_value))
                            return false;
                    }
                }
            }
        }

        if (opt_value.isUsed())
            ++i;
    }

    return true;
}

}
