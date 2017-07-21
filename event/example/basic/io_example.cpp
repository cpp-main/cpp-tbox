#include <unistd.h>
#include <iostream>
#include <tbox/event/loop.h>
#include <tbox/event/fd_item.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void FdCallback(int fd, short event)
{
    char input_buff[200];
    int rsize = read(fd, input_buff, sizeof(input_buff));
    input_buff[rsize - 1] = '\0';
    cout << "fd: " << fd << " INPUT is [" << input_buff << "]" << endl;
}

int main()
{
    Loop* sp_loop = Loop::New();
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    FdItem* sp_fd = sp_loop->newFdItem();
    sp_fd->initialize(STDIN_FILENO, FdItem::kReadEvent, Item::Mode::kPersist);
    using std::placeholders::_1;
    sp_fd->setCallback(std::bind(FdCallback, STDIN_FILENO, _1));
    sp_fd->enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_fd;
    delete sp_loop;
    return 0;
}
