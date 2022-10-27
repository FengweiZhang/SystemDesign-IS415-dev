#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("fg");
MODULE_DESCRIPTION("hook try");
MODULE_VERSION("0.1");

// End: Kernel Module

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>


// define function pointer 
typedef void (* sys_call_ptr_t)(void);

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

sys_call_ptr_t *sys_call_ptr = NULL;


static sys_call_ptr_t* get_sys_call_table(void)
{
    // cannot be used  in kernels > 5.7
    // sys_call_ptr_t *ret = NULL;
    // ret = (sys_call_ptr_t *)kallsyms_lookup_name("sys_call_table");

    return (sys_call_ptr_t*)kp.addr;
}


static int test_hook_init(void)
{
    register_kprobe(&kp);
    // sys_call_ptr = get_sys_call_table();
    // printk("sys_call_ptr: 0x%lu\n", *sys_call_ptr);
    printk("Found at 0x%px \n", get_sys_call_table());
    // try yi xia
    // try liangxia
    


    return 0;
}


static void test_hook_exit(void)
{
    unregister_kprobe(&kp);
}   

module_init(test_hook_init);
module_exit(test_hook_exit);
