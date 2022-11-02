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
static pid_t pid = -1;      // user space server pid (pid_t is int)
static atomic_t index;      // prm_msg 消息 index

/**
 * @brief 向用户态程序发送消息以查询权限是否满足
 * 
 * @return int 
 */
int check_rights(void)
{
    struct prm_msg msg;
    struct sem_msg *ptr = NULL;

    // 设置共享内存，初始化对应的信息量，设置共享内存状态为ready
    ptr = kmalloc(sizeof(struct sem_msg), GFP_KERNEL);
    memset(ptr, 0, sizeof(struct sem_msg));
    ptr->status = SEM_STATUS_READY;
    sema_init(&(ptr->sem), 0);
    
    // 向内核态程序发送查询消息
    msg.index = atomic_inc_return(&index);
    msg.type = PRM_MSG_TYPE_CHECK;
    msg.sem_msg_ptr = (u64)ptr;
    k2u_send((char *)&msg, sizeof(struct sem_msg));
    // 等待返回消息
    down(&(ptr->sem));
    // dowm(&(ptr->sem), SEM_WAIT_CYCLE) // 在SEM_WAIT_CYCLE个时钟周期内等待信号量

    kfree(ptr);
    if(ptr->status == SEM_STATUS_LOADED)
    {
        return (int)(ptr->data);
    }
    return PRM_ERROR;
}


/**
 * @brief Send len bytes data in buf to user space process.
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

    printk("%s %s: Send netlink msg to user space!\n", module_name, name);
    return nlmsg_unicast(netlink_socket, skb_out, pid);
}



/**
 * @brief message receive handle function
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

    printk("%s %s: Netlink msg recieved!\n", module_name, name);


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

    }
    else if(ptr->type == PRM_MSG_TYPE_DISCONNECT)
    {
        // 用户态断开连接
        // 这里没有检查是否是对应的程序要求断开连接，直接断开
        printk("%s %s: Disconnection message received from pid=%d, current connection pid=%d\n",
            module_name, name, msg->nlh.nlmsg_pid, pid);
        pid = -1;
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
        printk("%08x\n", *(((u32 *)ptr)+0));
        printk("%08x\n", *(((u32 *)ptr)+1));
        printk("%08x\n", *(((u32 *)ptr)+2));
        printk("%016llx\n", *(u64 *)(((u32 *)ptr)+3));        
    }


    

    kfree(buf);
}


/**
 * @brief Create netlink socket
 * 
 * @return If this function succeeds, return 0, -1 indicates an error.
 */
static int k2u_socket_create(void)
{
    // set message receive callback function
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
    printk("%s %s: Socket Create Succeed!\n", module_name, name);

    // 初始化 index
    atomic_set(&index, 0);

    return PRM_SUCCESS;

}


/**
 * @brief Close netlink socket
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


int prm_netlink_init(void)
{
    return k2u_socket_create();
}


int prm_netlink_exit(void)
{
    return k2u_socket_close();
}

