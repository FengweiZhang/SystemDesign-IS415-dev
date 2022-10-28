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

struct prm_nlmsg {
    struct nlmsghdr nlh;
    uint32_t   msg_len;
    uint8_t    msg_data[PAYLOAD_MAX_SIZE];
};

// End: Same in both kernel mode and user mode

int u2k_socket_init();
int u2k_socket_release();

int u2k_send(char *buf, size_t len);
ssize_t u2k_recv(char *buf, size_t buflen);


#endif