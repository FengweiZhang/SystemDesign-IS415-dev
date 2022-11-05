/**
 * @file server.c
 * @author 刘梓池 (liuzichi@sjtu.edu.cn)
 * @brief 服务器端程序，运行在后台。
 * 需要以root用户权限运行。连接数据库和内核模块。
 * @date 2020-11-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#define _GNU_SOURCE

#include <sqlite3.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../common/prm_error.h"
#include "database.h"
#include "operation.h"

// 服务器使用的socket路径
#define SOCK_PATH "/tmp/server.socket"

sqlite3 *db;
int req_len, rsp_len, rc, server_sock, client_sock;

/**
 * @brief 向服务器发送的请求结构
 * 设置用户或文件的权限，设置用户控制网络的权限，设置用户控制IO的权限
 * op | 操作
 * 1  | 设置用户的等级
 * 2  | 设置文件的等级
 */
struct req
{
    int op;
    int level;
    unsigned long uid; // uid
    unsigned long ino; //文件号
} reqbuf;

/**
 * @brief 服务器回复的数据结构
 * stat | 含义
 * 4    | 文件主与当前用户不匹配
 * 2    | 文件已经位于文件箱(insert)或者不在文件箱内(delete)
 * 1    | 数据库操作错误
 * 0    | 一切正常
 */
struct rsp
{
    int stat;
    int level;
} rspbuf;

/**
 * @brief 暂时用于调试，返回成功状态
 */
void success()
{
    rspbuf.stat = 1;
    send(client_sock, &rspbuf, rsp_len, 0);
}

void setUserLevel(char *uid, int level)
{
    printf("set user %s,level: %d \n", uid, level);
    int ret = db_set_right(db, "user_file", uid, level);
    if (ret == 0)
    {
        printf("success\n");
        rspbuf.stat = OP_SUCCESS;
    }
    else
    {
        printf("fail\n");
        rspbuf.stat = OP_FAIL;
    }

    send(client_sock, &rspbuf, rsp_len, 0);
}

void getUserLevel(char *uid)
{
    printf("search user %s level\n", uid);
    int level = db_search_right(db, "user_file", uid);
    if (level == -1)
    {
        rspbuf.stat = OP_NOT_FIND;
    }
    else
    {
        rspbuf.stat = OP_SUCCESS;
        rspbuf.level = level;
    }

    send(client_sock, &rspbuf, rsp_len, 0);
}

void deleteUserLevel(char *uid)
{
    int ret = db_delete_right(db, "user_file", uid);
    if (ret == 0)
    {
        rspbuf.stat = OP_SUCCESS;
    }
    else
    {
        rspbuf.stat = OP_FAIL;
    }

    send(client_sock, &rspbuf, rsp_len, 0);
}

void setFileLevel(char *inode, int level)
{
    int ret = db_set_right(db, "file", inode, level);
    if (ret == 0)
    {
        rspbuf.stat = OP_SUCCESS;
    }
    else
    {
        rspbuf.stat = OP_FAIL;
    }

    send(client_sock, &rspbuf, rsp_len, 0);
}

void getFileLevel(char *inode)
{
    int level = db_search_right(db, "user_file", inode);
    if (level == -1)
    {
        rspbuf.stat = OP_NOT_FIND;
    }
    else
    {
        rspbuf.stat = OP_SUCCESS;
        rspbuf.level = level;
    }

    send(client_sock, &rspbuf, rsp_len, 0);
}

void deleteFileLevel(char *inode)
{
    int ret = db_delete_right(db, "file", inode);
    if (ret == 0)
    {
        rspbuf.stat = OP_SUCCESS;
    }
    else
    {
        rspbuf.stat = OP_FAIL;
    }

    send(client_sock, &rspbuf, rsp_len, 0);
}

/**
 * @brief 服务器端的主程序
 * 创建了一个子进程，父进程负责与内核模块的通信
 * 子进程负责与用户端进程之间的socket通信
 *
 * @param argc 命令行参数数量
 * @param argv 命令行参数首地址
 * @return int
 */
int main(int argc, char **argv)
{
    // 用于socket连接的变量并初始化
    int sockaddr_len, ucred_len;
    struct sockaddr_un server_sockaddr, client_sockaddr;

    req_len = sizeof(struct req);
    rsp_len = sizeof(struct rsp);
    sockaddr_len = sizeof(struct sockaddr_un);
    memset(&server_sockaddr, 0, sockaddr_len);
    memset(&client_sockaddr, 0, sockaddr_len);
    // 用户凭证
    struct ucred cr;
    ucred_len = sizeof(struct ucred);

    //连接数据库
    rc = sqlite3_open(DATABASE_PATH, &db);
    if (rc)
    {
        printf("%s\n", "SQLITE OPEN ERROR");
        sqlite3_close(db);
        exit(1);
    }

    // 父进程使用netlink与内核模块通信
    if (fork())
    {
        while (1)
        {
        }
        // // netlink协议使用sockaddr_nl地址
        // struct sockaddr_nl src_sockaddr, dest_sockaddr;
        // struct nlmsghdr * nlh = NULL;
        // struct msghdr msg;
        // struct iovec iov;

        // // 创建地址并初始化
        // nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(unsigned long)));
        // memset(& src_sockaddr, 0, sizeof(struct sockaddr_nl));
        // memset(& dest_sockaddr, 0, sizeof(struct sockaddr_nl));
        // memset(nlh, 0, NLMSG_SPACE(sizeof(unsigned long)));
        // memset(& msg, 0, sizeof(struct msghdr));

        // // 创建netlink的socket
        // server_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_SAFE);
        // // 创建用户态地址，pid需要设置为进程的pid
        // // 实际上是一个socket标识，不同线程可以设置为不同的值
        // // groups为多播组，设置为0表示不加入多播
        // src_sockaddr.nl_family = AF_NETLINK;
        // src_sockaddr.nl_pid = getpid();
        // src_sockaddr.nl_groups = 0;
        // // 绑定socket和地址
        // bind(server_sock, (struct sockaddr *)& src_sockaddr, sizeof(struct sockaddr_nl));
        // // 设置核心态用户地址，核心态的pid必须设置为0
        // dest_sockaddr.nl_family = AF_NETLINK;
        // dest_sockaddr.nl_pid = 0;
        // dest_sockaddr.nl_groups = 0;
        // // 设置netlink socket的信息头部
        // nlh -> nlmsg_len = NLMSG_SPACE(sizeof(unsigned long));
        // nlh -> nlmsg_pid = getpid();
        // nlh -> nlmsg_flags = 0;
        // // 设置iov 可以把多个信息通过一次系统调用发送
        // iov.iov_base = (void *)nlh;
        // iov.iov_len = NLMSG_SPACE(sizeof(unsigned long));
        // // 设置接收地址
        // msg.msg_name = (void *)& dest_sockaddr;
        // msg.msg_namelen = sizeof(struct sockaddr_nl);
        // msg.msg_iov = & iov;
        // msg.msg_iovlen = 1;

        // // 填充并发送初始化就绪数据
        // * (unsigned long *)NLMSG_DATA(nlh) = (unsigned long)0xffffffff << 32;
        // sendmsg(server_sock, & msg, 0);
        // while (1) {
        //     // 接收内核态的信息
        //     recvmsg(server_sock, & msg, 0);
        //     // 查询指定文件的属主
        //     snprintf(sql, 63, SELECT2, * (unsigned long *)NLMSG_DATA(nlh));
        //     * (unsigned long *)NLMSG_DATA(nlh) = 0;
        //     sqlite3_exec(db, sql, callback_get_fileowner_or_check, (uid_t *)NLMSG_DATA(nlh), NULL);
        //     // 将查询结果发送到内核
        //     sendmsg(server_sock, & msg, 0);
        // }

        // // 关闭socket和数据库，释放内存
        // close(server_sock);
        // free(nlh);
        // sqlite3_close(db);
    }
    // 子进程使用socket与客户端之间通信
    else
    {
        // ext2fs_init();

        // 创建服务器socket
        server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_sock == -1)
        {
            printf("%s\n", "SOCKET ERROR");
            exit(1);
        }

        // 连接服务器socket文件
        server_sockaddr.sun_family = AF_UNIX;
        strcpy(server_sockaddr.sun_path, SOCK_PATH);
        unlink(SOCK_PATH);
        rc = bind(server_sock, (struct sockaddr *)&server_sockaddr, sockaddr_len);
        if (rc == -1)
        {
            printf("%s\n", "BIND ERROR");
            close(server_sock);
            exit(1);
        }

        // 等待连接
        chmod(SOCK_PATH, 0666);
        rc = listen(server_sock, 16);
        if (rc == -1)
        {
            printf("%s\n", "LISTEN ERROR");
            close(server_sock);
            exit(1);
        }

        while (1)
        {
            // 接受客户端socket连接
            client_sock = accept(server_sock, (struct sockaddr *)&client_sockaddr, &sockaddr_len);
            if (client_sock == -1)
            {
                close(client_sock);
                continue;
            }
            // 接受客户端发送的请求数据
            rc = recv(client_sock, &reqbuf, req_len, 0);
            if (rc == -1)
            {
                close(client_sock);
                continue;
            }
            // SOL_SOCKET表示socket级别（不变）
            // SO_PEERCRED表示获取对方的身份凭证
            if (getsockopt(client_sock, SOL_SOCKET, SO_PEERCRED, &cr, &ucred_len) == -1)
            {
                close(client_sock);
                continue;
            }
            printf("receive operation: %d\n", reqbuf.op);
            char uid_ch[20], ino_ch[20];
            // 根据请求的类型进行处理
            switch (reqbuf.op)
            {
            case SET_USER_LEVEL:
                printf("set\n");
                printf("set user %lu,level: %d \n", reqbuf.uid, reqbuf.level);
                // 设置用户的级别
                sprintf(uid_ch, "%lu", reqbuf.uid);
                setUserLevel(uid_ch, reqbuf.level);
                break;
            case GET_USER_LEVEL:
                // 得到用户的级别
                sprintf(uid_ch, "%lu", reqbuf.uid);
                getUserLevel(uid_ch);
                break;
            case DELETE_USER_LEVEL:
                // 删除用户的级别
                sprintf(uid_ch, "%lu", reqbuf.uid);
                deleteUserLevel(uid_ch);
                break;
            case SET_FILE_LEVEL:
                // 设置文件的级别
                sprintf(ino_ch, "%lu", reqbuf.ino);
                setFileLevel(ino_ch, reqbuf.level);
                break;
            case GET_FILE_LEVEL:
                // 得到文件的级别
                sprintf(ino_ch, "%lu", reqbuf.ino);
                getFileLevel(ino_ch);
                break;
            case DELETE_FILE_LEVEL:
                // 删除文件的级别
                sprintf(ino_ch, "%lu", reqbuf.ino);
                deleteFileLevel(ino_ch);
                break;
            default:
                printf("some error");
            }
            // 传输结束以后关闭连接
            close(client_sock);
        }
        // 关闭socket和数据库连接
        close(server_sock);
        close(client_sock);
        // sqlite3_close(db);
    }

    return 0;
}
