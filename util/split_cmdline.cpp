#include "split_cmdline.h"

namespace tbox::util {

using namespace std;

bool SplitCmdline(const std::string &cmd, std::vector<std::string> &args)
{
    size_t start_pos = 0;
    size_t end_pos = 0;
    args.clear();

    while (true) {
        start_pos = cmd.find_first_not_of(" \t", end_pos);
        if (start_pos == std::string::npos)
            break;

        char start_char = cmd.at(start_pos);
        if (start_char == '\'' || start_char == '\"') {
            end_pos = cmd.find_first_of(start_char, start_pos + 1);
            if (end_pos == std::string::npos)
                return false;

            args.push_back(cmd.substr(start_pos + 1, end_pos - start_pos - 1));
            ++end_pos;
        } else {
            end_pos = cmd.find_first_of(" \t", start_pos);
            args.push_back(cmd.substr(start_pos, end_pos - start_pos));
            if (end_pos == std::string::npos)
                break;
        }
    }

    return true;
}

}
