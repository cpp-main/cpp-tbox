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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#include "helper.h"

#include <sstream>
#include <tbox/base/catch_throw.h>

namespace tbox {
namespace terminal {

NodeToken AddDirNode(TerminalNodes &terminal, NodeToken parent_node,
                     const std::string &name, const std::string &help)
{
    auto dir_node = terminal.createDirNode(help);
    if (terminal.mountNode(parent_node, dir_node, name))
        return dir_node;

    terminal.deleteNode(dir_node);
    return NodeToken();
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, VoidFunc &&func)
{
    auto func_node = terminal.createFuncNode(
        [func] (const Session &s, const Args &) {
            func();
            s.send("done\r\n");
        },
        "call function"
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, bool &value)
{
    auto func_node = terminal.createFuncNode(
        [&value] (const Session &s, const Args &a) {
            std::ostringstream oss;
            bool is_ok = false;
            if (a.size() == 1u) {
                is_ok = true;

            } else if (a.size() == 2u) {
                auto &str = a[1];
                if (str == "true" || str == "True" || str == "TRUE" ||
                    str == "on" || str == "On" || str == "ON") {
                    value = true;
                    is_ok = true;

                } else if (str == "false" || str == "False" || str == "FALSE" ||
                    str == "off" || str == "Off" || str == "OFF") {
                    value = false;
                    is_ok = true;
                }
            }

            if (is_ok) {
                oss << "value: " << (value ? "true" : "false") << "\r\n";
            } else {
                oss << "Usage: " << a[0] << "             # print value\r\n"
                    << "       " << a[0] << " true|false  # set value\r\n";
            }

            s.send(oss.str());
        },
        "print and set bool value"
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, std::string &value)
{
    auto func_node = terminal.createFuncNode(
        [&value] (const Session &s, const Args &a) {
            std::ostringstream oss;
            bool is_ok = false;
            if (a.size() == 1u) {
                is_ok = true;

            } else if (a.size() == 2u) {
                value = a[1];
                is_ok = true;
            }

            if (is_ok) {
                oss << "value: \"" << value << "\"\r\n";
            } else {
                oss << "Usage: " << a[0] << "              # print value\r\n"
                    << "       " << a[0] << " 'new value'  # set value\r\n";
            }

            s.send(oss.str());
        },
        "print and set string value"
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, int &value, int min_value, int max_value)
{
    auto func_node = terminal.createFuncNode(
        [&value, min_value, max_value] (const Session &s, const Args &a) {
            std::ostringstream oss;
            bool is_ok = false;
            if (a.size() == 1u) {
                is_ok = true;

            } else if (a.size() == 2u) {
                auto &str = a[1];
                int new_value = 0;
                if (!CatchThrowQuietly([&] { new_value = std::stoi(str); })) {
                    if (new_value >= min_value && new_value <= max_value) {
                        value = new_value;
                        is_ok = true;
                    } else {
                        oss << "Error: out of range[" << min_value << ',' << max_value << "]\r\n";
                    }
                } else {
                    oss << "Error: not number\r\n";
                }
            }

            if (is_ok) {
                oss << "value: " << value << "\r\n";
            } else {
                oss << "Usage: " << a[0] << "        # print value\r\n"
                    << "       " << a[0] << " number # set value\r\n";
            }

            s.send(oss.str());
        },
        "print and set integer value"
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, double &value, double min_value, double max_value)
{
    auto func_node = terminal.createFuncNode(
        [&value, min_value, max_value] (const Session &s, const Args &a) {
            std::ostringstream oss;
            bool is_ok = false;
            if (a.size() == 1u) {
                is_ok = true;

            } else if (a.size() == 2u) {
                auto &str = a[1];
                double new_value = 0;
                if (!CatchThrowQuietly([&] { new_value = std::stod(str); })) {
                    if (new_value >= min_value && new_value <= max_value) {
                        value = new_value;
                        is_ok = true;
                    } else {
                        oss << "Error: out of range[" << min_value << ',' << max_value << "]\r\n";
                    }
                } else {
                    oss << "Error: not number\r\n";
                }
            }

            if (is_ok) {
                oss << "value: " << value << "\r\n";
            } else {
                oss << "Usage: " << a[0] << "        # print value\r\n"
                    << "       " << a[0] << " number # set value\r\n";
            }

            s.send(oss.str());
        },
        "print and set double value"
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, const StringFuncNodeProfile &profile)
{
    auto func_node = terminal.createFuncNode(
        [profile] (const Session &s, const Args &a) {
            std::ostringstream oss;
            bool is_ok = false;

            if (a.size() == 1u && profile.get_func) {
                oss << '"' << profile.get_func() << "\"\r\n";
                is_ok = true;

            } else if (a.size() == 2u && profile.set_func) {
                auto &str = a[1];
                if (profile.set_func(str)) {
                    oss << "done.\r\n";
                    is_ok = true;
                } else {
                    oss << "fail.\r\n";
                }
            }

            if (!is_ok) {
                if (profile.usage.empty()) {
                    oss << "Usage:\r\n";

                    if (profile.get_func)
                        oss << "  " << a[0] << "               # print string\r\n";

                    if (profile.set_func)
                        oss << "  " << a[0] << " 'new string'  # set string\r\n";

                } else {
                    oss << profile.usage;
                }
            }

            s.send(oss.str());
        },
        profile.help.empty() ? "print or set string" : profile.help
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, const BooleanFuncNodeProfile &profile)
{
    auto func_node = terminal.createFuncNode(
        [profile] (const Session &s, const Args &a) {
            std::ostringstream oss;
            bool is_ok = false;

            if (a.size() == 1u && profile.get_func) {
                oss << (profile.get_func() ? "true" : "false") << "\r\n";
                is_ok = true;

            } else if (a.size() == 2u && profile.set_func) {
                auto &str = a[1];
                bool new_value = false;
                if (str == "true" || str == "True" || str == "TRUE" ||
                    str == "on" || str == "On" || str == "ON") {
                    new_value = true;
                    is_ok = true;

                } else if (str == "false" || str == "False" || str == "FALSE" ||
                    str == "off" || str == "Off" || str == "OFF") {
                    new_value = false;
                    is_ok = true;
                }

                if (is_ok) {
                    if (profile.set_func(new_value)) {
                        oss << "done.\r\n";
                    } else {
                        oss << "fail.\r\n";
                        is_ok = false;
                    }
                }
            }

            if (!is_ok) {
                if (profile.usage.empty()) {
                    oss << "Usage:\r\n";

                    if (profile.get_func)
                        oss << "  " << a[0] << "         # print boolean\r\n";

                    if (profile.set_func)
                        oss << "  " << a[0] << " on|off  # set boolean\r\n";

                } else {
                    oss << profile.usage;
                }
            }

            s.send(oss.str());
        },
        profile.help.empty() ? "print and set bool value" : profile.help
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, const IntegerFuncNodeProfile &profile) {
    auto func_node = terminal.createFuncNode(
        [profile] (const Session &s, const Args &a) {
            std::ostringstream oss;
            bool is_ok = false;

            if (a.size() == 1u && profile.get_func) {
                oss << profile.get_func() << "\r\n";
                is_ok = true;

            } else if (a.size() == 2u && profile.set_func) {
                auto &str = a[1];
                int new_value = 0;
                CatchThrowQuietly([&] { new_value = std::stoi(str); is_ok = true; });
                if (is_ok) {
                    if (new_value < profile.min_value || new_value > profile.max_value) {
                      oss << "fail, out of range.\r\n";
                      is_ok = false;
                    }
                }

                if (is_ok) {
                    if (profile.set_func(new_value)) {
                        oss << "done.\r\n";
                    } else {
                        oss << "fail.\r\n";
                        is_ok = false;
                    }
                }
            }

            if (!is_ok) {
                if (profile.usage.empty()) {
                    oss << "Usage:\r\n";

                    if (profile.get_func)
                        oss << "  " << a[0] << "           # print integer\r\n";

                    if (profile.set_func) {
                        oss << "  " << a[0] << " <number>  # set integer";
                        if (profile.min_value != std::numeric_limits<int>::min() &&
                            profile.min_value != std::numeric_limits<int>::max()) {
                            oss << ", range: [" << profile.min_value << ',' << profile.max_value << ']';
                        } else if (profile.min_value != std::numeric_limits<int>::min()) {
                            oss << ", range: >=" << profile.min_value;
                        } else if (profile.max_value != std::numeric_limits<int>::max()) {
                            oss << ", range: <=" << profile.max_value;
                        }
                        oss << "\r\n";
                    }

                } else {
                    oss << profile.usage;
                }
            }

            s.send(oss.str());
        },
        profile.help.empty() ? "print and set integer value" : profile.help
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, const DoubleFuncNodeProfile &profile) {
    auto func_node = terminal.createFuncNode(
        [profile] (const Session &s, const Args &a) {
            std::ostringstream oss;
            bool is_ok = false;

            if (a.size() == 1u && profile.get_func) {
                oss << profile.get_func() << "\r\n";
                is_ok = true;

            } else if (a.size() == 2u && profile.set_func) {
                auto &str = a[1];
                double new_value = 0;
                CatchThrowQuietly([&] { new_value = std::stod(str); is_ok = true; });
                if (is_ok) {
                    if (new_value < profile.min_value || new_value > profile.max_value) {
                      oss << "fail, out of range.\r\n";
                      is_ok = false;
                    }
                }

                if (is_ok) {
                    if (profile.set_func(new_value)) {
                        oss << "done.\r\n";
                    } else {
                        oss << "fail.\r\n";
                        is_ok = false;
                    }
                }
            }

            if (!is_ok) {
                if (profile.usage.empty()) {
                    oss << "Usage:\r\n";

                    if (profile.get_func)
                        oss << "  " << a[0] << "           # print double\r\n";

                    if (profile.set_func) {
                        oss << "  " << a[0] << " <double>  # set double";
                        if (profile.min_value != std::numeric_limits<int>::min() &&
                            profile.min_value != std::numeric_limits<int>::max()) {
                            oss << ", range: [" << profile.min_value << ',' << profile.max_value << ']';
                        } else if (profile.min_value != std::numeric_limits<int>::min()) {
                            oss << ", range: >=" << profile.min_value;
                        } else if (profile.max_value != std::numeric_limits<int>::max()) {
                            oss << ", range: <=" << profile.max_value;
                        }
                        oss << "\r\n";
                    }

                } else {
                    oss << profile.usage;
                }
            }

            s.send(oss.str());
        },
        profile.help.empty() ? "print and set double value" : profile.help
    );

    terminal.mountNode(parent_node, func_node, name);
    return func_node;
}

}
}
