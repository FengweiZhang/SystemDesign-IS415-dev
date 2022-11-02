#ifndef _PRM_NETLINK_H
#define _PRM_NETLINK_H

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

int prm_netlink_init(void);
int prm_netlink_exit(void);

int k2u_send(char *buf, size_t len);

#endif