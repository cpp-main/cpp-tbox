#include "argument_parser.h"

namespace tbox::util {

bool ArgumentParser::parse(int argc, const char * const * const argv)
{
    for (int i = 1; i < argc; ++i) {
        const std::string curr = argv[i];
        OptStr next;
        if (i != (argc - 1))
            next.set(argv[i + 1]);

        if (curr[0] == '-') {
            if (curr[1] == '-') {   //! 匹配 --xxxx
                const std::string opt = curr.substr(2);
                //!FIXME: need handle --xyz=abc
                if (!handler_(0, opt, next))
                    return false;
            } else {    //! match -x
                for (size_t j = 1; j < curr.size(); ++j) {
                    char opt = curr[j];
                    if (j != (curr.size() - 1)) {
                        OptStr tmp;
                        if (!handler_(opt, "", tmp))
                            return false;
                    } else {
                        if (!handler_(opt, "", next))
                            return false;
                    }
                }
            }
        }

        if (next.isUsed())
            ++i;
    }

    return true;
}

}
