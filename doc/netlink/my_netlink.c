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

// Begin: Kernel Moduel
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("fg");
MODULE_DESCRIPTION("netlink try");
MODULE_VERSION("0.0");

// End: Kernel Module

#include "../common/prm_error.h"


#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <linux/slab.h>


// Begin: Same in both kernel mode and user mode

#define NETLINK_PRM         30
#define PAYLOAD_MAX_SIZE    1024

struct prm_nlmsg {
    struct nlmsghdr nlh;
    u32   msg_len;
    u8    msg_data[PAYLOAD_MAX_SIZE];
};

// End: Same in both kernel mode and user mode

struct sock *netlink_socket = NULL;
pid_t pid = -1;     // pid is int

/**
 * @brief Send len bytes in buf to user space process.
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
    strncpy((char *)NLMSG_DATA(nlh)+4, buf, len);

    printk("Send msg to user space!\n");
    return nlmsg_unicast(netlink_socket, skb_out, pid);
}


/**
 * @brief message receive handle function
 *  
 */
static void netlink_message_handle(struct sk_buff *skb)
{
    u8 *buf = NULL;

    // get prm_nlmsg
    struct prm_nlmsg *msg = (struct prm_nlmsg *) skb->data;
    // set pid
    pid = msg->nlh.nlmsg_pid;
    // get data, store in buf
    
    buf = kmalloc(PAYLOAD_MAX_SIZE, GFP_KERNEL);
    strncpy(buf, (char *)msg->msg_data, (size_t)msg->msg_len);

    printk("Netlink info get!\n");

    // handle function
    // @param buf, msg_len 
    k2u_send(buf, msg->msg_len);

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

    netlink_socket = (struct sock *)netlink_kernel_create(&init_net, NETLINK_PRM, &cfg);

    if(netlink_socket == NULL)
    {
        printk("Socket Create Failed!\n");
        return PRM_ERROR;
    }
    printk("Socket Create Succeed!\n");
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
    printk("Socket Release Succeed!\n");
    return PRM_SUCCESS;
}


// Begin: Kernel Module
static int test_netlink_init(void)
{
    k2u_socket_create();
    return PRM_SUCCESS;
}

static void test_netlink_exit(void)
{
    k2u_socket_close();
}

module_init(test_netlink_init);
module_exit(test_netlink_exit);

// End: Kernel Module




