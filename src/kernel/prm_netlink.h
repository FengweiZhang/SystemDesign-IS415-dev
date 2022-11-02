#ifndef _PRM_NETLINK_H
#define _PRM_NETLINK_H

#include "../common/prm_error.h"

#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <asm/atomic.h>

// Begin: Same in both kernel mode and user mode

#define NETLINK_PRM         30
#define PAYLOAD_MAX_SIZE    1024

// 在netlink标准消息的基础上添加的基础消息结构
struct prm_nlmsg {
    struct nlmsghdr nlh;
    u32   msg_len;
    u8    msg_data[PAYLOAD_MAX_SIZE];
};

// 用户态与核心态之间发送的消息的结构
struct prm_msg {
    s32     index;      // 在模块中使用atomic_t的值，为了减少处理，取值范围是signed int
    u32     type;   
    s32     result_type;
    u64     sem_msg_ptr;
};

// prm_msg的type的取值
#define PRM_MSG_TYPE_CONNECT            (u32)0x00000001     // 用户态向核心态进行注册
#define PRM_MSG_TYPE_CONNECT_CONFIRM    (u32)0x00000002     // 核心态向用户态发送注册成功消息
#define PRM_MSG_TYPE_CHECK              (u32)0x00000003     // 核心态向用户态发起权限查询请求
#define PRM_MSG_TYPE_RESULT             (u32)0x00000004     // 权限查询结果: 
#define PRM_MSG_TYPE_DISCONNECT         (u32)0x00000005     // 用户态取消连接

// prm_msg result_type 取值
#define CHECK_RESULT_NOTPASS            (s32)(1)        // 无权访问
#define CHECK_RESULT_PASS               (s32)(2)        // 有权访问

// End: Same in both kernel mode and user mode

#define SEM_WAIT_CYCLE      1000

// 用户内核态进程间数据传递的结构体
struct sem_msg {
    u32     status; 
    s32     data;           // 取值与prm_msg.result_type一致
    struct semaphore    sem;
};

// sem_msg的status取值
#define SEM_STATUS_READY    (u32)0x0f0f0f0f     // 初始化完成，等待写入数据
#define SEM_STATUS_LOADED   (u32)0xf0f0f0f0     // 已经写入数据，等待读取


int prm_netlink_init(void);
int prm_netlink_exit(void);

int k2u_send(char *buf, size_t len);

int check_rights(void);



#endif