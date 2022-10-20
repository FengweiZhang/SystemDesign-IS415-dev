/**
 * @file my_user.c
 * @author fg (you@domain.com)
 * @brief netlink moduel in user mode
 * @version 0.1
 * @date 2022-10-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../common/prm_error.h"

#include <stdio.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Begin: Same in both kernel mode and user mode

#define NETLINK_PRM         30
#define PAYLOAD_MAX_SIZE    1024

struct prm_nlmsg {
    struct nlmsghdr nlh;
    __u32   msg_len;
    __u8    msg_data[PAYLOAD_MAX_SIZE];
};

// End: Same in both kernel mode and user mode

int netlink_socket = -1;
struct sockaddr_nl  *user_addr = NULL;      // self address
struct sockaddr_nl  *kernel_addr = NULL;    // target address
struct prm_nlmsg    *msg = NULL;            // message buffer


/**
 * @brief create netlink socket, record address information
 * 
 * @return If this function succeeds, return PRM_SUCCESS, else return PRM_ERROR
 */
int u2k_socket_init()
{
    // Create netlink socket
    netlink_socket = (AF_NETLINK, SOCK_RAW, NETLINK_PRM);
    if (netlink_socket == -1)
    {
        perror("Socket create failed!\n");
        return PRM_ERROR;
    }
    
    // debug point
    if(sizeof(struct prm_nlmsg) != NLMSG_SPACE(1024+4)) perror("nlmsg space error\n");
    
    // fill msg head info
    msg = (struct prm_nlmsg *)malloc(sizeof(struct prm_nlmsg));
    if(msg == NULL)
    {
        perror("msg malloc failed!\n");
        return PRM_ERROR;
    }
    memset(msg, 0, sizeof(struct prm_nlmsg));
    msg->nlh.nlmsg_len = sizeof(struct prm_nlmsg);
    msg->nlh.nlmsg_pid = getpid();  // sender's pid
    msg->nlh.nlmsg_flags = 0;   // no special flag

    // fill sender's addr
    user_addr = (struct sockaddr_nl *)malloc(sizeof(struct sockaddr_nl));
    if(user_addr == NULL)
    {
        perror("user_addr malloc failed!\n");
        return PRM_ERROR;
    }
    memset(user_addr, 0, sizeof(struct sockaddr_nl));
    user_addr->nl_family = AF_NETLINK;   // Netlink socket
    user_addr->nl_pid = getpid();        // user space process pid
    user_addr->nl_groups = 0;            // no multicast

    // fill receive's addr
    kernel_addr = (struct sockaddr_nl *)malloc(sizeof(struct sockaddr_nl *));
    if(kernel_addr == NULL)
    {
        perror("kernel_addr malloc failed!\n");
        return PRM_ERROR;
    }
    kernel_addr->nl_family = AF_NETLINK;
    kernel_addr->nl_pid = 0;     // kernel process pid
    kernel_addr->nl_groups = 0;

    // bind socket
    bind(netlink_socket, (struct sockaddr *)& user_addr, sizeof(struct sockaddr_nl));
}

/**
 * @brief close socket and release address information
 * 
 * @return PRM_SUCCESS
 */
int u2k_socket_release()
{
    close(netlink_socket);
    free(msg);
    free(user_addr);
    free(kernel_addr);

    netlink_socket = -1;
    msg = NULL;
    user_addr = NULL;
    kernel_addr = NULL;

    return PRM_SUCCESS;
}

/**
 * @brief send len bytes in buf to kernel module
 * 
 * @param buf 
 * @param len the num of bytes to be sent
 * @return PRM_SUCCESS for success, PRM_ERROR for error.
 */
int u2k_send(char *buf, size_t len)
{
    if(buf == NULL)
    {
        perror("u2k send buf is NULL!\n");
        return PRM_ERROR;
    }
    if(len > PAYLOAD_MAX_SIZE)
    {
        perror("u2k send too long msg!\n");
        return PRM_ERROR;
    }

    memset(&(msg->msg_len), 0, PAYLOAD_MAX_SIZE+4);
    strncpy(msg->msg_data, buf, len);
    msg->msg_len = len;

    int len = sendto(netlink_socket, msg, msg->nlh.nlmsg_len, 0, (struct sockaddr *)&kernel_addr, sizeof(struct sockaddr_nl));
    if(len == -1)
    {
        perror("u2k send failed!\n");
        return PRM_ERROR;
    }
    return PRM_SUCCESS;
}

/**
 * @brief receive msg from kernel module, save data in buf
 * 
 * @param buf 
 * @param buflen length of buf
 * @return If this function succeeds, return num of received bytes, else return PRM_ERROR
 */
int u2k_recv(char *buf, size_t buflen)
{
    if(buf == NULL)
    {
        perror("u2k recv buf is NULL!\n");
        return PRM_ERROR;
    }

    size_t len = -1;
    struct prm_nlmsg msg;
    // recv msg
    recvfrom(netlink_socket, &msg, sizeof(struct prm_nlmsg), 0, (struct sockaddr *)&kernel_addr, &len);

    if(len == -1)
    {
        perror("u2k recv failed!\n");
        return PRM_ERROR;
    }
    if(buflen < msg.msg_len)
    {
        perror("u2k recv failed, buf is too short!\n");
        return PRM_ERROR;
    }

    strncpy(buf, msg.msg_data, msg.msg_len);

    return len;
}

