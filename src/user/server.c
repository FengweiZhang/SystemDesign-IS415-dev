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

#include "user_netlink.h"
#include "socket_error.h"
#include "database.h"
#include "operation.h"
#include "log.h"

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
    logwrite(info, "set user %s,level: %d", uid, level);
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
    logwrite(info, "search user %s level", uid);
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
    printf("set file %s,level: %d \n", inode, level);
    logwrite(info, "set file %s,level: %d", inode, level);
    int ret = db_set_right(db, "file", inode, level);
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

void getFileLevel(char *inode)
{
    printf("set file %s level\n", inode);
    logwrite(info, "set file %s level", inode);
    int level = db_search_right(db, "file", inode);
    if (level == -1)
    {
        printf("fail\n");
        rspbuf.stat = OP_NOT_FIND;
    }
    else
    {
        printf("success\n");
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

void setOtherLevel(char *uid, char *inode, int level)
{
    char uid_ino[30];
    sprintf(uid_ino, "%s.%s", uid, inode);
    printf("set uid_ino %s, level: %d \n", uid_ino, level);
    logwrite(info, "set other %s,uid %s, level: %d", inode, uid, level);
    int ret = 0;
    if (level > 0)
    {
        printf("level:%d\n", level);
        ret = db_set_right(db, "user_file", uid_ino, 1);
    }
    else
    {
        ret = db_set_right(db, "user_file", uid_ino, 0);
    }

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

void getOtherLevel(char *uid, char *inode)
{
    char uid_ino[30];
    sprintf(uid_ino, "%s.%s", uid, inode);
    printf("get uid_ino %s\n", uid_ino);
    logwrite(info, "get other %s,uid %s", inode, uid);
    int level = db_search_right(db, "user_file", uid_ino);
    if (level == -1)
    {
        printf("fail\n");
        rspbuf.stat = OP_NOT_FIND;
    }
    else
    {
        printf("success\n");
        rspbuf.stat = OP_SUCCESS;
        rspbuf.level = level;
    }

    send(client_sock, &rspbuf, rsp_len, 0);
}

void deleteOtherLevel(char *uid, char *inode)
{
    char uid_ino[30];
    sprintf(uid_ino, "%s.%s", uid, inode);
    printf("delete uid_ino %s\n", uid_ino);
    logwrite(info, "delete other %s,uid %s", inode, uid);
    int ret = db_delete_right(db, "user_file", uid_ino);
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
        if (argc > 1)
        {
            while(1){}
        }
        else
        {
        char buf[1024];
        char msg[1024];
        
        // scanf("%s", msg);
        u2k_socket_init();
        printf("init succees\n");

        // scanf("%s", msg);
        u2k_connect();
        printf("connect!\n");

        while(1)
        {   
            u2k_recv(buf, 1024);
            printf("rece msg\n");
            msg_handle((struct prm_msg *)buf, db);
            printf("handel finish\n");
        }

        scanf("%s", msg);
        u2k_disconnect();
        printf("disconnect");

        // scanf("%s", msg);
        u2k_socket_release();
        printf("Release!");
        }
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
            logwrite(info, "receive operation: %d\n", reqbuf.op);
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
                printf("set file %lu,level: %d \n", reqbuf.ino, reqbuf.level);
                sprintf(ino_ch, "%lu", reqbuf.ino);
                setFileLevel(ino_ch, reqbuf.level);
                break;
            case GET_FILE_LEVEL:
                // 得到文件的级别
                printf("get file %lu level\n", reqbuf.ino);
                sprintf(ino_ch, "%lu", reqbuf.ino);
                getFileLevel(ino_ch);
                break;
            case DELETE_FILE_LEVEL:
                // 删除file的级别
                sprintf(ino_ch, "%lu", reqbuf.ino);
                deleteFileLevel(ino_ch);
                break;
            case DELETE_USER_TO_OTHER_LEVEL:
                // 删除other的级别
                sprintf(uid_ch, "%lu", reqbuf.uid);
                sprintf(ino_ch, "%lu", reqbuf.ino);
                deleteOtherLevel(uid_ch, ino_ch);
                break;
            case SET_USER_TO_OTHER_LEVEL:
                // 设置other的级别
                printf("set other %lu,uid: %lu, level: %d \n", reqbuf.ino, reqbuf.uid, reqbuf.level);
                sprintf(ino_ch, "%lu", reqbuf.ino);
                sprintf(uid_ch, "%lu", reqbuf.uid);
                setOtherLevel(uid_ch, ino_ch, reqbuf.level);
                break;
            case GET_USER_TO_OTHER_LEVEL:
                // 得到other的级别
                printf("get other %lu,uid: %lu\n", reqbuf.ino, reqbuf.uid);
                sprintf(ino_ch, "%lu", reqbuf.ino);
                sprintf(uid_ch, "%lu", reqbuf.uid);
                getOtherLevel(uid_ch, ino_ch);
                break;
            default:
                sprintf(ino_ch, "%lu", reqbuf.ino);
                printf("some error");
                logwrite(error, "%s", "some error");
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
