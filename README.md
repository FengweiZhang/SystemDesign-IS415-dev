# IS415 System Software Desgin Dev

本项目是SJTU IS415 2022-2023学年课程大作业

本项目实现了一个基于强制访问控制的集成运行权限管理程序。

本项目在 Ubuntu 20.04.01 操作系统上实现，内核版本号为 5.15.0-xx-generic

由于在 linux 系统中，进程会继承用户的权限，因此本系统通过对用户的权限设置实现对于进程的权限管理。

本项目实现的功能如下：

1.  进程的文件、目录访问权限控制
    -   用户分为 4 个权限等级：1, 2, 3, 4；文件与目录分为四个权限等级：1, 2, 3, 4；
    -   权限等级数值越大，权限等级越高；
    -   用户进程可以访问权限等级相当和权限等级较低的文件与目录。
2.  进程的网络权限控制
    -   对用户进程的网络访问权限进行控制
3.  root 进程的 reboot 权限控制
    -   对 root 进程的 reboot 系列权限进行控制
4.  用户进程的系统内核日志权限控制
    -   对用户进程的系统内核日志访问、日志删除等权限进行控制
5.  日志记录
    -   对于越权行为输出系统内核日志
    -   对于权限控制输出记录日志


```bash
SystemDesign-IS415-dev
├── doc/			# 项目开发过程中的文档与代码资料
├── README.md
└── src/			# 项目源代码
    ├── kernel/	 		# 内核态源代码
    │   ├── Makefile
    │   ├── prm_error.h		# 内核态通用返回值定义
    │   ├── prm_hook.c		# 内核态hook功能函数库
    │   ├── prm_hook.h
    │   ├── prm_module.c	# 内核态模块主文件
    │   ├── prm_netlink.c	# 内核态netlink功能函数库
    │   └── prm_netlink.h
    └── user/		# 用户态源代码
        ├── Makefile
        ├── server.c		# 后端服务器
        ├── client.c		# CLI客户端
        ├── operation.h			# client操作类型定义
        ├── socket_error.h		# client返回值定义
        ├── database.c				# 数据库功能函数库
        ├── database.h				
        ├── database.db				# 数据库文件
        ├── databaseExtension.c		 # 数据库权限查询功能函数
        ├── databaseExtension.h
        ├── log/				# 日志文件目录
        ├── log.c				# 日志功能函数库
        ├── log.h				
        ├── log.conf			# 日志配置
        ├── prm_error.h			# 同src/kernel/prm_error.h
        ├── user_netlink.c		# 用户态netlink功能函数
        ├── user_netlink.h
        └── test/	# 测试用函数、文件与脚本
```