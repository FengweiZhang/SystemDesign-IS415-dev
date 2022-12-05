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
    // .symbol_name = "kallsyms_lookup_name"
    .symbol_name = "sys_call_table"
};

/**
 * @brief Get the sys call table position
 * 
 * @return sys_call_ptr_t* 
 */
static sys_call_ptr_t* get_sys_call_table(void)
{
    // function kallsyms_lookup_name() cannot be used in kernels > 5.7
    // sys_call_ptr_t *ret = NULL;
    // ret = (sys_call_ptr_t *)kallsyms_lookup_name("sys_call_table");

    return (sys_call_ptr_t*)kp.addr;
}

typedef asmlinkage long (*sys_openat_t)(struct pt_regs*);

sys_openat_t real_openat;

asmlinkage long my_sys_openat(struct pt_regs * reg)
{
    printk("Hook success\n");
    return real_openat(reg);
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

unsigned long kaddr_lookup_name(const char *fname_raw)
{
    int i;
    unsigned long kaddr;
    char *fname_lookup, *fname;

    fname_lookup = kvzalloc(NAME_MAX, GFP_KERNEL);
    if (!fname_lookup)
        return 0;

    fname = kvzalloc(strlen(fname_raw)+4, GFP_KERNEL);
    if (!fname)
        return 0;

    /*
    * We have to add "+0x0" to the end of our function name
    * because that's the format that sprint_symbol() returns
    * to us. If we don't do this, then our search can stop
    * prematurely and give us the wrong function address!
    */
    strcpy(fname, fname_raw);
    strcat(fname, "+0x0");

    /*
        * Get the kernel base address:
        * sprint_symbol() is less than 0x100000 from the start of the kernel, so
        * we can just AND-out the last 3 bytes from it's address to the the base
        * address.
        * There might be a better symbol-name to use?
        */
    kaddr = (unsigned long) &sprint_symbol;
    kaddr &= 0xffffffffff000000;

    /*
        * All the syscalls (and all interesting kernel functions I've seen so far)
        * are within the first 0x100000 bytes of the base address. However, the kernel
        * functions are all aligned so that the final nibble is 0x0, so we only
        * have to check every 16th address.
    */
    for ( i = 0x0 ; i < 0x200000 ; i++ )
    {
        /*
            * Lookup the name ascribed to the current kernel address
        */
        sprint_symbol(fname_lookup, kaddr);

        /*
        * Compare the looked-up name to the one we want
        */
        if ( strncmp(fname_lookup, fname, strlen(fname)) == 0 )
        {
            /*
                * Clean up and return the found address
            */
            kvfree(fname_lookup);
            return kaddr;
        }
        /*
        * Jump 16 addresses to next possible address
        */
        kaddr += 0x10;
    }
    /*
        * We didn't find the name, so clean up and return 0
        */
    kvfree(fname_lookup);
    return 0;
}


static int test_hook_init(void)
{
    // get syscall table position
    register_kprobe(&kp);
    sys_call_ptr = get_sys_call_table();
    // printk("sys_call_table found at 0x%px \n", sys_call_ptr);

    // unsigned long addr = kaddr_lookup_name("sys_call_table");
    // printk("addr %px\n", addr);
    // sys_call_ptr = (sys_call_ptr_t *)addr;

    write_protection_off();
    real_openat = (void *)sys_call_ptr[__NR_openat];
    sys_call_ptr[__NR_openat] = (sys_call_ptr_t)my_sys_openat;
    write_protection_on();

    
    // printk("Yes\n");
    // printk("sys_table_ptr %px\n", sys_call_ptr);
    // printk("sys_openat %px\n", sys_call_ptr[__NR_openat]);
    // printk("my_sys_openat %px\n", my_sys_openat);


    return 0;
}


static void test_hook_exit(void)
{

    write_protection_off();
    sys_call_ptr[__NR_openat] = (sys_call_ptr_t)real_openat;
    write_protection_on();

    unregister_kprobe(&kp);
}   

module_init(test_hook_init);
module_exit(test_hook_exit);
