#include "async.h"

#include <memory>
#include <tbox/util/fs.h>

namespace tbox {
namespace async {

namespace {
}

Async::Async(eventx::ThreadPool &thread_pool)
    : thread_pool_(thread_pool)
{ }

void Async::readFile(const std::string &filename, ErrorCodeStringCb &&cb)
{
    struct Tmp {
        int error_code = 0;
        std::string result;
        ErrorCodeStringCb cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_.execute(
        [tmp, filename] {
            if (!util::fs::ReadStringFromTextFile(filename, tmp->result))
                tmp->error_code = 1;
        },
        [tmp] {
            tmp->cb(tmp->error_code, tmp->result);
        }
    );
}

void Async::readFileLines(const std::string &filename, ErrorCodeStringVecCb &&cb)
{
    struct Tmp {
        int error_code = 0;
        std::vector<std::string> line_vec;
        ErrorCodeStringVecCb cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_.execute(
        [tmp, filename] {
            auto is_succ = util::fs::ReadEachLineFromTextFile(filename,
                [tmp](const std::string &line) {
                    tmp->line_vec.emplace_back(line);
                }
            );

            if (!is_succ)
                tmp->error_code = 1;
        },
        [tmp] {
            tmp->cb(tmp->error_code, tmp->line_vec);
        }
    );
}

void Async::writeFile(const std::string &filename, const std::string &content, bool sync_now, ErrorCodeCb &&cb)
{
    struct Tmp {
        int error_code = 0;
        ErrorCodeCb cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_.execute(
        [tmp, filename, content, sync_now] {
            if (!util::fs::WriteStringToTextFile(filename, content, sync_now))
                tmp->error_code = 1;
        },
        [tmp] {
            tmp->cb(tmp->error_code);
        }
    );
}

void Async::appendFile(const std::string &filename, const std::string &content, bool sync_now, ErrorCodeCb &&cb)
{
    struct Tmp {
        int error_code = 0;
        ErrorCodeCb cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_.execute(
        [tmp, filename, content, sync_now] {
            if (!util::fs::AppendStringToTextFile(filename, content, sync_now))
                tmp->error_code = 1;
        },
        [tmp] {
            tmp->cb(tmp->error_code);
        }
    );
}

}
}
