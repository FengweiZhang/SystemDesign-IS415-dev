#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("fg");
MODULE_DESCRIPTION("hook try");
MODULE_VERSION("0.1");

// End: Kernel Module

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/syscalls.h>
#include <asm/unistd_64.h>


#include "../common/prm_error.h"

// define a function pointer 
typedef void (* sys_call_ptr_t)(void);

// system call table pointer 
sys_call_ptr_t *sys_call_ptr = NULL;

// a struct used to get sys call table position
static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

/**
 * @brief Get the sys call table position
 * 
 * @return sys_call_ptr_t* 
 */
static sys_call_ptr_t* get_sys_call_table(void)
{
    // cannot be used  in kernels > 5.7
    // sys_call_ptr_t *ret = NULL;
    // ret = (sys_call_ptr_t *)kallsyms_lookup_name("sys_call_table");

    return (sys_call_ptr_t*)kp.addr;
}

typedef asmlinkage long (*sys_open_t)(const char __user *filename, umode_t mode);

sys_open_t real_chmod;

asmlinkage long my_sys_chmod(const char __user *filename, umode_t mode)
{
    printk("Hook success");
    return real_chmod(filename, mode);
}

extern unsigned long __force_order ;
inline void write_cr0_new(unsigned long cr0) 
{
    __asm__ volatile("mov %0,%%cr0" : "+r"(cr0), "+m"(__force_order));
}


static int test_hook_init(void)
{
    // get syscall table position
    register_kprobe(&kp);
    sys_call_ptr = get_sys_call_table();
    // printk("sys_call_table found at 0x%px \n", sys_call_ptr);

    // real_chmod = (void *)sys_call_ptr[__NR_chmod];
    // sys_call_ptr[__NR_chmod] = (sys_call_ptr_t)my_sys_chmod;

    unsigned long cr0 = read_cr0();
    printk("CR0: %016lx", cr0);

    write_cr0_new(read_cr0() & (~0x10000));
    printk("disable: %016lx", read_cr0());

    write_cr0_new(read_cr0() | (0x10000));
    printk("enable: %016lx", read_cr0());

    return 0;
}


static void test_hook_exit(void)
{

    // sys_call_ptr[__NR_chmod] = (sys_call_ptr_t)real_chmod;

    unregister_kprobe(&kp);
}   

module_init(test_hook_init);
module_exit(test_hook_exit);
