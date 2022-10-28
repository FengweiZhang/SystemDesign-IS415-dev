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
#include <asm/ptrace.h>


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

typedef asmlinkage long (*sys_read_t)(struct pt_regs * reg);

sys_read_t real_read;

asmlinkage long my_sys_read(struct pt_regs * reg)
{
    printk("Hook success");
    return real_read(reg);
}

// change linux kernel memory write protection
extern unsigned long __force_order ;
inline void write_cr0_new(unsigned long cr0) 
{
    asm volatile("mov %0,%%cr0" : "+r"(cr0), "+m"(__force_order));
}
// write protextion off
void write_protection_off(void)
{
    write_cr0_new(read_cr0() & (~0x10000));
}
// write protection on
void write_protection_on(void)
{
    write_cr0_new(read_cr0() | 0x10000);
}

static int test_hook_init(void)
{
    // get syscall table position
    register_kprobe(&kp);
    sys_call_ptr = get_sys_call_table();
    // printk("sys_call_table found at 0x%px \n", sys_call_ptr);

    write_protection_off();
    real_read = (void *)sys_call_ptr[__NR_read];
    sys_call_ptr[__NR_read] = (sys_call_ptr_t)my_sys_read;
    write_protection_on();

    return 0;
}


static void test_hook_exit(void)
{

    write_protection_off();
    sys_call_ptr[__NR_read] = (sys_call_ptr_t)real_read;
    write_protection_on();

    unregister_kprobe(&kp);
}   

module_init(test_hook_init);
module_exit(test_hook_exit);
