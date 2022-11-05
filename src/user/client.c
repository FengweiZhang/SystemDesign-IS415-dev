/**
 * @file client.c
 * @author 刘梓池 (liuzichi@sjtu.edu.cn)
 * @brief CLI客户端
 * @date 2022-10-20
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>

#include "../common/prm_error.h"
#include "operation.h"

// Unix Domain Socket通信使用的socket文件路径
#define SERVER_PATH "/tmp/server.socket"
#define CLIENT_PATH "/tmp/client.%u.socket"

/**
 * @brief 向服务器发送的请求结构
 * 设置用户或文件的权限，设置用户控制网络的权限，设置用户控制IO的权限
 * op | 操作
- 0:设置用户等级
- 1：查看用户等级
- 2: 删除用户等级
- 3：设置文件等级
- 4：查看文件等级
- 5：删除文件等级
- 6：设置用户对设备/网络等的权限
- 7：查看用户对设备/网络等的权限
- 8：删除用户对设备/网络等的权限
 */
struct req
{
    unsigned char op;
    unsigned char level;
    unsigned long uid; // uid
    unsigned long ino; //文件号,或设备、网络代表号
};

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
    unsigned char stat;
    unsigned char level;
};

// 文件路径
char filepath[4092];
unsigned char username[255];

/**
 * @brief 客户端的主要处理函数
 * 与服务器建立连接，发送请求并获得服务器的返回数据
 *
 * @param op 操作类型
 * @param ino 文件的inode节点号
 */
void handle(unsigned char op, unsigned long ino, unsigned long uid, unsigned char level)
{
    int client_sock, rc, sockaddr_len;
    // 这里使用的是sockaddr_un表示Unix域套接字地址
    // 通常在internet传输中使用sockaddr_in表示互联网域地址
    struct sockaddr_un server_sockaddr, client_sockaddr;
    struct req reqbuf = {op, level, uid, ino};
    struct rsp rspbuf;

    // 清空socket地址
    sockaddr_len = sizeof(struct sockaddr_un);
    memset(&server_sockaddr, 0, sockaddr_len);
    memset(&client_sockaddr, 0, sockaddr_len);

    // 创建Unix域的socket
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock == -1)
    {
        printf("%s\n", "SOCKET ERROR");
        exit(1);
    }

    // 创建客户端socket文件并绑定到套接字
    client_sockaddr.sun_family = AF_UNIX;
    snprintf(client_sockaddr.sun_path, 107, CLIENT_PATH, getuid());
    unlink(client_sockaddr.sun_path); // 防止上次使用忘记删除
    rc = bind(client_sock, (struct sockaddr *)&client_sockaddr, sockaddr_len);
    if (rc == -1)
    {
        printf("%s\n", "BIND ERROR");
        close(client_sock);
        exit(1);
    }

    // 连接到服务器端socket文件
    // 与Internet不同，直接使用connect连接到一个已经打开的socket文件
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SERVER_PATH);
    rc = connect(client_sock, (struct sockaddr *)&server_sockaddr, sockaddr_len);
    if (rc == -1)
    {
        printf("%s\n", "CONNECT ERROR");
        close(client_sock);
        exit(1);
    }

    printf("send message");
    // 发送请求
    rc = send(client_sock, &reqbuf, sizeof(struct req), 0);
    if (rc == -1)
    {
        printf("%s\n", "SEND ERROR");
        close(client_sock);
        exit(1);
    }

    // 接受数据并处理结果, 错误处理
    rc = recv(client_sock, &rspbuf, sizeof(struct rsp), 0);
    printf("receive message");

    if (rc == -1)
    {
        printf("%s\n", "RECV ERROR");
        close(client_sock);
        exit(1);
    }

    // 处理结果
    switch (rspbuf.stat)
    {
    case OP_SUCCESS:
        printf("operation success!");
        break;
    case OP_FAIL:
        printf("operation fail!");
        break;
    case OP_NOT_FIND:
        printf("do not find user or file level!") break;
    default:
        printf("some error!");
        break;
    }

    if (op == GET_USER_LEVEL)
    {
        printf("get user level : %c", level);
    }

    if (op == GET_FILE_LEVEL)
    {
        printf("get file level : %c", level);
    }

    // 关闭socket 删除文件
    close(client_sock);
    // 使用结束后需要删除
    unlink(client_sockaddr.sun_path);
}

/**
 * @brief 显示CLI客户端的命令行参数和用法
 *
 */
void usage(void)
{
    printf("%s\n", "Usage: fvault [OPTION]... [FILE]...\n\n"
                   "  -s (set)	set level\n"
                   "  -g (get)	get level\n"
                   "  -d (delete)	delete level\n"
                   "  -u (user)	input user name for operation\n"
                   "  -f (file)	input file path for operation\n"
                   "  -l (level)	set level\n");
}

/**
 * @brief 主函数，程序入口
 *
 * @param argc 命令行参数数量
 * @param argv 命令行参数的首地址数组
 * @return int
 */
int main(int argc, char **argv)
{
    int ch;
    unsigned char option = 0;
    unsigned char mode1 = -1; // u f u+f
    unsigned char mode2 = -1; // g/s/d
    unsigned char level = 0;
    unsigned long inode = 0;
    unsigned long uid = 0;
    struct passwd *user;
    struct stat file_stat;
    // 解析参数，如果一个连字符后面多个选项，只识别最后一个 f设置文件，u设置用户,l为level,s:set,d:delete,g:get
    // 例如：fvault -cl等价于fvault -l，循环解析
    while ((ch = getopt(argc, argv, "sgdf:u:l:")) != -1)
    {
        switch (ch)
        {
        case 's':
            if (mode2 != -1)
            {
                usage();
                return -1;
            }
            mode2 = 0;
            break;
        case 'g':
            if (mode2 != -1)
            {
                usage();
                return -1;
            }
            mode2 = 1;
            break;
        case 'd':
            if (mode2 != -1)
            {
                usage();
                return -1;
            }
            mode2 = 2;
            break;
        case 'f':
            mode1 += 2;
            memset(filepath, 0, 4096);
            memcpy(filepath, optarg, strlen(optarg));
            break;
        case 'u':
            mode1 += 1;
            memset(username, 0, 255);
            memcpy(username, optarg, strlen(optarg));
            break;
        case 'l':
            level = optarg[0];
            // level只能是特定的几个值，否则调用usage
            break;
        default:
            usage();
            return -1;
        }
    }

    option = mode1 * 3 + mode2;

    // 只有root可以设置权限
    if (getuid() != 0)
    {
        printf("Operation not Permitted\n");
        return 0;
    }
    // 对应文件来说不存在要报错
    if (mode1 == 1)
    {
        // 检查文件状态并处理
        if (!stat(filepath, &file_stat))
        {
            // 获得文件的inode节点号
            inode = file_stat.st_ino;
        }
        else
        {
            printf("%s: No such file or directory\n", filepath);
        }
    }
    // 对于用户名来说不存在要报错
    if (mode1 == 0)
    {
        user = getpwnam(username);
        if (user != NULL)
        {
            uid = user->pw_uid;
        }
        else
        {
            printf("%s: No such user\n", username);
        }
    }

    printf("start handle");
    handle(option, inode, uid, level);

    return 0;
}
