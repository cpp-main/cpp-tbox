# 是什么？

terminal 是一个提供与运行中程序类似shell交互命令终端的模块。

# 为什么需要它？

在服务程序运行时，通常只会通过日志输出程序的执行过程。开发或运维人员无法与它进行直接的交互。
比如以下场景：

* 在开发过程中，工程师还没有来得及实现完整的界面交互，但又想让程序模块执行某个动作；
* 当程序运行异常的时候，工程师非常希望能打印关键的信息方便排查；
* 在程序遇到突发状态时，运维人员希望在不停止服务的前提下，调整运行参数。

有了 terminal，开发或运维工程师可直接与程序通过命令的形式进行交互。
只需要 telnet 登陆进去，通过命令的方式让它执行指定的函数，即可满足上述场景的需求。
从而减少工程师的调试与运维工程量。

terminal 的设计模仿 Bash。
命令的组织非常类似文件系统的目录树：

```
# tree
|-- dir1
|   |-- dir1_1
|   |   |-- async*
|   |   `-- root(R)
|   `-- dir1_2
|       `-- sync*
|-- dir2
`-- sync*
```

支持 cd, ls, tree, pwd, history, !n, !-n, !! 等常用命令；
还支持常用的按键动作 UP, DOWN, LEFT, RIGHT, DELETE, HOME, END。

# 目前有哪里 telnet 客户端通过了测试？

* Windows 自带 telnet 命令
* Linux 终端 telnet 命令
* Putty telnet 连接
* XShell telnet 连接
* Tabby telnet profile (注意：Input mode 要设置为 Normal)

