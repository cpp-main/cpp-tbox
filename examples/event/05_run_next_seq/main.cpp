#include <iostream>
#include <tbox/event/loop.h>

using namespace std;
using namespace tbox::event;

void PrintUsage(const char *process_name)
{
    cout << "Usage:" << process_name << " epoll" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    Loop* sp_loop = Loop::New(argv[1]);
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    sp_loop->runInLoop(
        [sp_loop] {
            cout << "A" << endl;
            sp_loop->runInLoop( [] { cout << "B" << endl; });
            sp_loop->runNext( [] { cout << "C" << endl; } );
            sp_loop->runNext( [] { cout << "D" << endl; } );
            cout << "a" << endl;
        }
    );

    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_loop;
    return 0;
}
