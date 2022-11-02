#ifndef _USER_NETLINK_H
#define _USER_NETLINK_H


#include "../common/prm_error.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Begin: Same in both kernel mode and user mode

#define NETLINK_PRM         30
#define PAYLOAD_MAX_SIZE    1024

// 在netlink标准消息的基础上添加的基础消息结构
struct prm_nlmsg {
    struct nlmsghdr nlh;
    uint32_t   msg_len;
    uint8_t    msg_data[PAYLOAD_MAX_SIZE];
};

// 用户态与核心态之间发送的消息的结构
struct prm_msg {
    int32_t     index;      // 在模块中使用atomic_t的值，为了减少处理，取值范围是signed int
    uint32_t    type;   
    int32_t     result_type;
    uint64_t    sem_msg_ptr;
};

// prm_msg的type的取值
#define PRM_MSG_TYPE_CONNECT            (uint32_t)0x00000001     // 用户态向核心态进行注册
#define PRM_MSG_TYPE_CONNECT_CONFIRM    (uint32_t)0x00000002
#define PRM_MSG_TYPE_CHECK              (uint32_t)0x00000003     // 核心态向用户态发起权限查询请求
#define PRM_MSG_TYPE_RESULT             (uint32_t)0x00000004     // 权限查询结果: 
#define PRM_MSG_TYPE_DISCONNECT         (uint32_t)0x00000005     // 用户态取消连接

// prm_msg result_type 取值
#define CHECK_RESULT_NOTPASS            (int32_t)(-1)       // 无权访问
#define CHECK_RESULT_PASS               (int32_t)(0)        // 有权访问

// End: Same in both kernel mode and user mode

int u2k_socket_init();
int u2k_socket_release();

int u2k_send(char *buf, size_t len);
ssize_t u2k_recv(char *buf, size_t buflen);


#endif