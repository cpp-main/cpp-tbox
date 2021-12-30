# 是什么？
coroutine 是基于 event 架构的协程库。它帮忙开发者处理异步模型不擅长的顺序型业务逻辑。

# 为什么需要协程
基于事件驱动的程序模型擅长处理 *“当发生xx事件，就做yy动作”* 的逻辑。 
如果事件之间是相互孤立的，那么就非常好处理。如果有简单的依赖，则需要加状态机进行处理。

一旦遇到了顺序型的业务逻辑，如：*“先做A动作，然后做B动作。如果A与B动作中任意一个的不成功，则做C动作，否则循环做D动作 ...”* 
基于事件驱动的程序实现起来就需要设计一个非常复杂的状态机，业务代码非常零散且难于维护。  
反面教材有 [Javascript 的回调地狱](http://callbackhell.com/)，非常恐怖。
![Javascript的回调地狱](https://pic1.zhimg.com/v2-cf1c78890006e078a538842a0caa7127_1440w.jpg?source=172ae18b)

解决方案有二：
1. 线程
2. 协程

线程的不足：
- 比较重，线程是CPU进行任务调度的单元，为每个业务创建一个线程占CPU且耗内存；
- 线程之间的切换不可控；
- 资源抢占不易管理。

协程的优点：
- 轻量，只需要为每个协程分配一个栈即可，栈大小可指定；
- 协程之间切换，由程序自行控制，使用 yield(), wait() 主动切换，易调试；
- 无资源抢占问题。

# 有哪些功能？
详见 scheduler.h 的接口定义。

# 怎么使用？
详见各单元测试用例。

## 基础用法
```c++
int main()
{
    using namespace tbox::event;
    using namespace tbox::coroutine;

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    //! 定义协程1的行为
    int routine1_count = 0;
    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 20; ++i) {
            ++routine1_count;
            sch.yield();    //! 主动让出执行权
        }
    };

    //! 定义协程2的行为
    int routine2_count = 0;
    auto routine2_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 10; ++i) {
            ++routine2_count;
            sch.yield();    //! 主动让出执行权
        }
    };

    sch.create(routine1_entry); //! 创建协程1，并开始执行
    sch.create(routine2_entry); //! 创建协程2，并开始执行

    sp_loop->exitLoop(chrono::seconds(1));
    sp_loop->runLoop();

    return 0;
}
```

## 辅助组件
为了让协程更好用，特设计了多种辅助组件。 
组件有：
- Channel，通道，多个协程等待数据，当有数据进入管道时最早等待的协程被唤醒，类似于 golang 的 ch；
- Semaphore，信号量，与多线程间的sem类似；
- Mutex，互斥量，与多线程间的mutex类似；
- Condition，条件量，当多个条件同时满足或满足任意时唤醒协程；
- Broadcast，广播，多个协程等待一个信号，当该信号发出时，所有等待的协程都被唤醒。

具体使用方法，见各组件的单元测试用例。
