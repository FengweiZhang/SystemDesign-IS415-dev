/**
 * @file my_netlink.c
 * @author fg (you@domain.com)
 * @brief netlink module in kernel mode
 * @version 0.1
 * @date 2022-10-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "prm_netlink.h"

extern char* module_name;  // kernel module name

static char *name = "Netlink";
static struct sock *netlink_socket = NULL;  // netlink socket
static pid_t pid = -1;      // user space server pid (pid_t is int)，同时是连接的状态
static atomic_t index;      // prm_msg 消息 index
static int fail_count = 0;

/**
 * @brief 查询权限信息
 * 
 * @param ino inode号
 * @param uid 用户uid
 * @param type 权限类型
 * @param result 存储查询结果的指针
 * @return int 查询操作成功与否
 */
int check_privilege(unsigned long ino, uid_t uid, int p_type, int *result)
{
    struct prm_msg msg;
    struct sem_msg *ptr = NULL;
    int down_ret = -1;

    // 检查用户态服务器是否已经连接
    if(pid==-1)
    {
        // 未连接
        return PRM_ERROR_SERVEROFFLINE;
    }

    // 设置共享内存，初始化对应的信息量，设置共享内存状态为ready
    ptr = kmalloc(sizeof(struct sem_msg), GFP_KERNEL);
    memset(ptr, 0, sizeof(struct sem_msg));
    ptr->status = SEM_STATUS_READY;
    sema_init(&(ptr->sem), 0);

    // 构建消息
    msg.index = atomic_inc_return(&index);
    msg.type = PRM_MSG_TYPE_CHECK;
    msg.ino = (u32)ino;
    msg.uid = (u32)uid;
    msg.p_type = (s32)p_type;
    msg.sem_msg_ptr = (u64)ptr;

    // if(p_type == P_DEMESG) printk("Check rights: dmesg uid=%u!\n", uid);
    // if(p_type == P_NET) printk("Check rights: net uid=%u!\n", uid);
    // if(p_type == P_REBOOT) printk("Check rights: reboot uid=%u!\n", uid);
    // if (p_type == P_STDIN) printk("Check rights: STDIN uid=%u\n", uid);
    // if (p_type == P_STDOUT) printk("Check rights: STDOUT uid=%u\n", uid);
    // if (p_type == P_STDERR) printk("Check rights: STDERR uid=%u\n", uid);
    // if (p_type == P_REG) printk("Check rights: REG file uid=%u inode=%ld\n", uid, ino);

    // 向内核态程序发送查询消息
    k2u_send((char *)&msg, sizeof(struct sem_msg));
    // 等待返回消息
    // down(&(ptr->sem));
    down_ret = down_timeout(&(ptr->sem), SEM_WAIT_CYCLE); // 在SEM_WAIT_CYCLE个时钟周期内等待信号量
    if(down_ret != 0)
    {
        fail_count += 1;
        printk("%s %s: Cannot get response from pid=%d\n", module_name, name, pid);
        if(fail_count >= 3)
        {
            // 多次失败取消连接
            printk("%s %s: Disconnection caused by failure, pid=%d\n", module_name, name, pid);
            // 重置连接状态
            pid = -1;
            fail_count = 0;
            printk("%s %s: Connection was closed\n", module_name, name);
        }
    }

    kfree(ptr);
    if(ptr->status == SEM_STATUS_LOADED)
    {
        *result = ptr->data;
        return PRM_SUCCESS;
    }
    return PRM_ERROR;
}



/**
 * @brief 向用户态发送消息
 * 
 * @param buf   
 * @param len 
 * @return  
 */
int k2u_send(char *buf, size_t len)
{
    struct sk_buff *skb_out = nlmsg_new(PAYLOAD_MAX_SIZE+4, GFP_KERNEL);
    struct nlmsghdr *nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, PAYLOAD_MAX_SIZE+4, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    // fill data
    *(u32 *)NLMSG_DATA(nlh) = len;
    memcpy((char *)NLMSG_DATA(nlh)+4, buf, len);

    // printk("%s %s: Send netlink msg to user space!\n", module_name, name);
    return nlmsg_unicast(netlink_socket, skb_out, pid);
}



/**
 * @brief 用户态消息的处理函数
 *  
 */
static void netlink_message_handle(struct sk_buff *skb)
{
    u8 *buf = NULL;
    int len;
    struct prm_nlmsg *msg;
    struct prm_msg *ptr;

    buf = kmalloc(PAYLOAD_MAX_SIZE, GFP_KERNEL);
    memset(buf, 0, PAYLOAD_MAX_SIZE);
    // get prm_nlmsg
    msg = (struct prm_nlmsg *) skb->data;
    // get len
    len = msg->msg_len;
    // get data
    memcpy(buf, (char *)msg->msg_data, (size_t)msg->msg_len);

    // printk("%s %s: Netlink msg recieved!\n", module_name, name);

    // handle different msg
    // @param buf, msg->msg_len 
    ptr = (struct prm_msg *)buf;
    if(ptr->type == PRM_MSG_TYPE_CONNECT)
    {
        // 收到用户态进程注册消息
        // 设置通信的用户态进程pid
        pid = msg->nlh.nlmsg_pid;
        // 返回确认消息
        ptr->index = atomic_inc_return(&index);
        ptr->type = PRM_MSG_TYPE_CONNECT_CONFIRM;
        printk("%s %s: Connection message received from pid=%d\n",
            module_name, name, pid);
        printk("%s %s: Send connection confirm message to pid=%d\n",
            module_name, name, pid);
        k2u_send((char *)ptr, sizeof(struct prm_msg));
        fail_count = 0;

    }
    else if(ptr->type == PRM_MSG_TYPE_DISCONNECT)
    {
        // 用户态断开连接
        // 这里没有检查是否确实为对应的程序要求断开连接，直接断开
        printk("%s %s: Disconnection message received from pid=%d, current connection pid=%d\n",
            module_name, name, msg->nlh.nlmsg_pid, pid);
        // 重置连接状态
        pid = -1;
        fail_count = 0;
        printk("%s %s: Connection was closed\n", module_name, name);

    }
    else if(ptr->type == PRM_MSG_TYPE_RESULT)
    {
        // 收到权限查询结果
        struct sem_msg* sem_msg_ptr = (struct sem_msg*)(ptr->sem_msg_ptr);
        if(sem_msg_ptr->status == SEM_STATUS_READY)
        {
            // 正在等待结果
            sem_msg_ptr->status = SEM_STATUS_LOADED;
            sem_msg_ptr->data = ptr->result_type;
            up(&(sem_msg_ptr->sem));
        }
        else
        {
            // 出现了一些问题，忽视，不做处理
        }
        
    }
    else
    {
        // 出现了未知的消息类型
        printk("Unknown message type\n");
        // printk("%08x\n", *(((u32 *)ptr)+0));
        // printk("%08x\n", *(((u32 *)ptr)+1));
        // printk("%08x\n", *(((u32 *)ptr)+2));
        // printk("%016llx\n", *(u64 *)(((u32 *)ptr)+3));        
    }


    kfree(buf);
}


/**
 * @brief 创建netlink socket
 * 
 * @return If this function succeeds, return 0, -1 indicates an error.
 */
static int k2u_socket_create(void)
{
    // 设置消息处理函数
    struct netlink_kernel_cfg cfg = {
        .input = netlink_message_handle,
    };
    // 建立netlink socket
    netlink_socket = (struct sock *)netlink_kernel_create(&init_net, NETLINK_PRM, &cfg);
    if(netlink_socket == NULL)
    {
        printk("%s %s: [Error]Socket Create Failed!\n", module_name, name);
        return PRM_ERROR;
    }
    printk("%s %s: Socket Create Succeed! Waiting for server to connect!\n", module_name, name);

    // 初始化 index
    atomic_set(&index, 0);

    return PRM_SUCCESS;

}


/**
 * @brief 关闭netlink socket
 * 
 * @return 0
 */
static int k2u_socket_close(void)
{
    if (netlink_socket){
        netlink_kernel_release(netlink_socket);
        netlink_socket = NULL;
    }   
    printk("%s %s: Socket Release Succeed!\n", module_name, name);
    return PRM_SUCCESS;
}

/**
 * @brief netlink模块初始化
 * 
 * @return int 
 */
int prm_netlink_init(void)
{
    return k2u_socket_create();
}

/**
 * @brief netlink模块卸载
 * 
 * @return int 
 */
int prm_netlink_exit(void)
{
    return k2u_socket_close();
}

