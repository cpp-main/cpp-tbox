#include "split_cmdline.h"

namespace tbox::util {

using namespace std;

bool SplitCmdline(const std::string &cmd, std::vector<std::string> &args)
{
    string curr_str;
    bool in_single_quote = false;
    bool in_double_quote = false;

    args.clear();

    for (char ch : cmd) {
        if (ch == '\'') {
            if (in_single_quote)
                in_single_quote = false;
            else if (in_double_quote)
                curr_str.push_back(ch);
            else
                in_single_quote = true;
        } else if (ch == '\"') {
            if (in_double_quote)
                in_double_quote = false;
            else if (in_single_quote)
                curr_str.push_back(ch);
            else
                in_double_quote = true;
        } else if (ch == ' ') {
            if (in_single_quote || in_double_quote) {
                curr_str.push_back(ch);
            } else {
                if (!curr_str.empty())
                    args.push_back(move(curr_str));
            }
        } else {
            curr_str.push_back(ch);
        }
    }

    if (!curr_str.empty())
        args.push_back(move(curr_str));

    return !(in_single_quote || in_double_quote);
}

}
