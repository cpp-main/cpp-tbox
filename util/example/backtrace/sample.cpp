#include <signal.h>
#include <iostream>
#include <thread>
#include <tbox/util/backtrace.h>

void DoAnotherThing()
{
    *static_cast<int*>(nullptr) = 0;
}

void DoSomeThing()
{
    DoAnotherThing();
}

int main() {
    tbox::util::Backtrace::instance().submit({SIGINT, SIGSEGV});
    std::cout << "press ctrl+c exit." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    DoSomeThing();
    return 0;
}
