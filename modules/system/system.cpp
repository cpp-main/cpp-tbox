#include "system.h"

#include <memory>
#include <tbox/base/assert.h>
#include <tbox/util/fs.h>

namespace tbox {
namespace system {

System::System(eventx::ThreadPool *thread_pool)
    : thread_pool_(thread_pool)
{ }

void System::readFile(const std::string &filename, StringCallback &&cb)
{
    TBOX_ASSERT(cb != nullptr);

    struct Tmp {
        int error_code = 0;
        std::string result;
        StringCallback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, filename] {
            if (!util::fs::ReadStringFromTextFile(filename, tmp->result))
                tmp->error_code = 1;
        },
        [tmp] {
            tmp->cb(tmp->error_code, tmp->result);
        }
    );
}

void System::readFileLines(const std::string &filename, StringVecCallback &&cb)
{
    TBOX_ASSERT(cb != nullptr);

    struct Tmp {
        int error_code = 0;
        std::vector<std::string> line_vec;
        StringVecCallback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
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

void System::writeFile(const std::string &filename, const std::string &content, bool sync_now, Callback &&cb)
{
    struct Tmp {
        int error_code = 0;
        Callback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, filename, content, sync_now] {
            if (!util::fs::WriteStringToTextFile(filename, content, sync_now))
                tmp->error_code = 1;
        },
        [tmp] {
            if (tmp->cb)
                tmp->cb(tmp->error_code);
        }
    );
}

void System::appendFile(const std::string &filename, const std::string &content, bool sync_now, Callback &&cb)
{
    struct Tmp {
        int error_code = 0;
        Callback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, filename, content, sync_now] {
            if (!util::fs::AppendStringToTextFile(filename, content, sync_now))
                tmp->error_code = 1;
        },
        [tmp] {
            if (tmp->cb)
                tmp->cb(tmp->error_code);
        }
    );
}

}
}
