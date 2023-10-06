#include <sstream>
#include <tbox/event/timer_event.h>
#include <tbox/terminal/terminal.h>
#include <tbox/terminal/service/tcp_rpc.h>
#include <tbox/terminal/session.h>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::terminal;

NodeToken dir3_tmpdir_token;

void BuildNodes(TerminalNodes &term, Loop *wp_loop)
{
    /**
     * sync_func 就是同步执行的命令函数
     * 当他被执行时，只需要调用 Session 的 send() 方法就可以输出信息到终端
     */
    Func sync_func = \
        [](const Session &s, const Args &args) {
            std::stringstream ss;
            ss << "This is sync_func.\r\nArgs:\r\n";
            for (size_t i = 0; i < args.size(); ++i)
                ss << '[' << i << "]: " << args.at(i) << "\r\n";
            s.send(ss.str());
        };

    /**
     * async_func 是异步执行的命令函数
     * 它的结果打印会在命令函数执行完成之后，
     *
     * 这种情冲常用于异步事件中，比如某命令的动作是发送HTTP请求，将请求的结果打印出来
     * 执行命令时，命令只是发出请求就结束。而结果则是在后面则到返回结果，或检测到异常
     * 时才会输出。
     */
    Func async_func = \
        [=](const Session &s, const Args &args) {
            if (args.size() < 2) {
                s.send(std::string("Usage: ") + args[0] + " <name>\r\n");
                return;
            }

            auto name = args[1];
            //! 创建一个定时器，令其每秒打印
            auto sp_timer = wp_loop->newTimerEvent();
            sp_timer->initialize(std::chrono::seconds(1), Event::Mode::kPersist);
            sp_timer->setCallback(
                [=] {   //! 注意：这里用的是 =，而不是 & 。用意是捕获 s 的副本，而不是引用它。
                    if (!s.isValid()) { //! 可以检查 s 对应的 Session 是否有效，如果无效则可以不做任何事情
                        sp_timer->disable();
                        wp_loop->run([sp_timer] { delete sp_timer; });
                        return;
                    }
                    s.send(std::string("timer ") + name + " timeout\r\n");
                }
            );
            sp_timer->enable();
            s.send(std::string("timer ") + name + " start\r\n");
        };

/**
构建如下结点树:
|-- dir1
|   |-- dir1_1
|   |   |-- async*
|   |   `-- root(R)
|   `-- dir1_2
|       `-- sync*
|-- dir2
|-- dir3
|-- mount
|-- umount
`-- sync*
*/
    auto sync_func_token = term.createFuncNode(sync_func, "This is sync func");
    auto async_func_token = term.createFuncNode(async_func, "This is async func");

    auto dir1_token = term.createDirNode("This is dir1");
    auto dir2_token = term.createDirNode();
    auto dir3_token = term.createDirNode("Dynamic dir3");

    auto dir1_1_token = term.createDirNode();
    auto dir1_2_token = term.createDirNode();

    auto create_func_token = term.createFuncNode(
        [=, &term] (const Session &s, const Args &args) {
            dir3_tmpdir_token = term.createDirNode("Dynamic dir");
            term.mountNode(dir3_tmpdir_token, sync_func_token, "sync");
        },
        "create tmp/ node"
    );

    auto mount_func_token = term.createFuncNode(
        [=, &term] (const Session &s, const Args &args) {
            term.mountNode(dir3_token, dir3_tmpdir_token, "tmp");
        },
        "mount tmp/ to /dir3"
    );

    auto umount_func_token = term.createFuncNode(
        [=, &term] (const Session &s, const Args &args) {
            term.umountNode(dir3_token, "tmp");
        },
        "umount tmp/ from /dir3"
    );

    auto delete_func_token = term.createFuncNode(
        [=, &term] (const Session &s, const Args &args) {
            term.deleteNode(dir3_tmpdir_token);
            dir3_tmpdir_token.reset();
        },
        "delete tmp/ node"
    );

    term.mountNode(dir1_1_token, async_func_token, "async");
    term.mountNode(dir1_2_token, sync_func_token, "sync");

    term.mountNode(dir1_token, dir1_1_token, "dir1_1");
    term.mountNode(dir1_token, dir1_2_token, "dir1_2");

    term.mountNode(term.rootNode(), sync_func_token, "sync");
    term.mountNode(term.rootNode(), dir1_token, "dir1");
    term.mountNode(term.rootNode(), dir2_token, "dir2");
    term.mountNode(term.rootNode(), dir3_token, "dir3");

    term.mountNode(dir3_token, create_func_token, "create");
    term.mountNode(dir3_token, mount_func_token, "mount");
    term.mountNode(dir3_token, umount_func_token, "umount");
    term.mountNode(dir3_token, delete_func_token, "delete");

    term.mountNode(dir1_1_token, term.rootNode(), "root");  //! 循环引用
}
