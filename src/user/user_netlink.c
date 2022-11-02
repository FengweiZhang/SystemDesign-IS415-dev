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

#include "user_netlink.h"


static int netlink_socket = -1;
static struct sockaddr_nl  *user_addr = NULL;      // self address
static struct sockaddr_nl  *kernel_addr = NULL;    // target address
static struct prm_nlmsg    *msg = NULL;            // message buffer


/**
 * @brief create netlink socket, record address information
 * 
 * @return If this function succeeds, return PRM_SUCCESS, else return PRM_ERROR
 */
int u2k_socket_init()
{
    // Create netlink socket
    netlink_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_PRM);
    if (netlink_socket == -1)
    {
        perror("Socket create failed!\n");
        return PRM_ERROR;
    }
    
    // debug
    // if(sizeof(struct prm_nlmsg) != NLMSG_SPACE(1024+4)) perror("nlmsg space error\n");
    
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
    memset(kernel_addr, 0, sizeof(struct sockaddr_nl));
    kernel_addr->nl_family = AF_NETLINK;
    kernel_addr->nl_pid = 0;     // kernel process pid
    kernel_addr->nl_groups = 0;

    // bind socket
    int ret = bind(netlink_socket, (struct sockaddr *) user_addr, sizeof(struct sockaddr_nl));
    if (ret == -1)
    {
        perror("bind error\n");
        return PRM_ERROR;
    }
    return PRM_SUCCESS;
}

/**
 * @brief close socket and release address information
 * 
 * @return PRM_SUCCESS
 */
int u2k_socket_release()
{
    struct prm_msg mmm;
    memset((void *)&mmm, 0, sizeof(mmm));
    mmm.type = PRM_MSG_TYPE_DISCONNECT;
    u2k_send((char *)&mmm, sizeof(struct prm_msg));

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
 * @brief 重新与内核态建立连接
 * 
 * @return int 
 */
int u2k_socket_reconnect()
{
    u2k_socket_release();
    u2k_socket_init();

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
    memcpy(msg->msg_data, buf, len);
    msg->msg_len = len;

    ssize_t send_len = sendto(netlink_socket, msg, msg->nlh.nlmsg_len, 0, (struct sockaddr *)kernel_addr, sizeof(struct sockaddr_nl));
    // printf("=========\n");
    if(send_len == -1)
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
ssize_t u2k_recv(char *buf, size_t buflen)
{
    if(buf == NULL)
    {
        perror("u2k recv buf is NULL!\n");
        return PRM_ERROR;
    }

    ssize_t len = -1;
    struct prm_nlmsg recv_msg;
    // recv msg
    socklen_t kernel_addrlen = sizeof(struct sockaddr_nl);
    len = recvfrom(netlink_socket, &recv_msg, sizeof(struct prm_nlmsg), 0, (struct sockaddr *)kernel_addr, &kernel_addrlen);

    if(len == -1)
    {
        perror("u2k recv failed!\n");
        return PRM_ERROR;
    }
    if(buflen < recv_msg.msg_len)
    {
        perror("u2k recv failed, buf is too short!\n");
        return PRM_ERROR;
    }

    memcpy(buf, recv_msg.msg_data, recv_msg.msg_len);
    return recv_msg.msg_len;
}

int main ()
{
    char * buf = "123321";
    char msg[1024];

    struct prm_msg mmm;
    // memset(&mmm, 0, sizeof(mmm));
    mmm.index = 0;
    mmm.type = PRM_MSG_TYPE_CONNECT;

    for(int i=0;i<5;i++)
    {
        printf("%08x\n", *((uint32_t *)(&mmm)+i));
    }

    u2k_socket_init();
    printf("init succees\n");
    int rest = u2k_send(&mmm, sizeof(struct prm_msg));
    printf("start msg send %d, sizeof %d\n", rest,sizeof(struct prm_msg));
    
    struct prm_msg *rrr = NULL;
    ssize_t ret = u2k_recv(msg, 1024);
    rrr = msg;
    printf("Receive: %u", rrr->type);

    u2k_socket_release();
    return 0;
}