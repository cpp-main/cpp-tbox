#include <signal.h>
#include <iostream>
#include <thread>
#include <tbox/util/backtrace.h>

void DoSomeThing(int deep)
{
    if (deep == 0)
        *static_cast<int*>(nullptr) = 0;    //! 触发异常
    else
        DoSomeThing(deep - 1);
}

int main() {
    tbox::util::Backtrace::instance()
        .maxFrames(10)
        .skipFrames(2)
        .submit({SIGINT, SIGSEGV});

    std::cout << "sleep 5sec, press ctrl+c exit." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    DoSomeThing(40);
    return 0;
}
