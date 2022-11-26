#include "../common/prm_error.h"
#include "prm_hook.h"
#include "prm_netlink.h"

#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FengweiZhang");
MODULE_DESCRIPTION("Process rights manager");
MODULE_VERSION("0.0");

char *module_name = "PRM";
module_param(module_name, charp, S_IRUGO);
MODULE_PARM_DESC(module_name, "Module name");


static int prm_init(void)
{
    // prm_netlink_init();
    prm_hook_init();

    // check_rights();

    printk("%s: Kernel module installed!\n", module_name);
    return 0;
}

static void prm_exit(void)
{
    prm_hook_exit();
    // prm_netlink_exit();

    printk("%s: Kernel module removed!\n", module_name);
}

module_init(prm_init);
module_exit(prm_exit);
