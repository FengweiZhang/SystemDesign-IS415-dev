#include "prm_hook.h"
#include "../common/prm_error.h"

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/syscalls.h>
#include <asm/unistd_64.h>
#include <asm/ptrace.h>

// define a function pointer 
typedef void (* sys_call_ptr_t)(void);


extern char* module_name;               // kernel module name
extern unsigned long __force_order;     // used to configure register cr0

static char * name = "Hook";

sys_call_ptr_t *sys_call_ptr = NULL;    // system call table pointer 

// a struct used to get sys call table position
static struct kprobe kp = {
    // .symbol_name = "kallsyms_lookup_name"
    .symbol_name = "sys_call_table"
};



// change linux kernel memory write protection
inline void write_cr0_new(unsigned long cr0){ asm volatile("mov %0,%%cr0" : "+r"(cr0), "+m"(__force_order)); }
// write protextion off
static void write_protection_off(void){ write_cr0_new(read_cr0() & (~0x10000)); }
// write protection on
static void write_protection_on(void){ write_cr0_new(read_cr0() | 0x10000); }

/**
 * @brief Get the sys call table pointer
 * 
 * @return sys_call_ptr_t* 
 */
static sys_call_ptr_t* get_sys_call_table(void)
{
    // function kallsyms_lookup_name() cannot be used in kernels > 5.7
    // return (sys_call_ptr_t *)kallsyms_lookup_name("sys_call_table");

    return (sys_call_ptr_t*)kp.addr;
}


/*
 * rdi  | rsi   | rdx   | r10
 * 1st  | 2nd   | 3rd   | 4th 
 */


typedef asmlinkage long (*sys_call_t)(struct pt_regs*);

sys_call_t real_openat;
sys_call_t real_read;
sys_call_t real_write;

asmlinkage long my_sys_openat(struct pt_regs * regs)
{
    // printk("Hook success\n");
    return real_openat(regs);
}

asmlinkage long my_sys_read(struct pt_regs * regs)
{
    // printk("Hook succeed %lu\n", regs->di);
    return real_read(regs);
}

asmlinkage long my_sys_write(struct pt_regs * regs)
{
    // printk("Hook succeed %lu\n", regs->di);
    unsigned int fd = regs->di;
    if (fd != 0 && fd != 1 && fd != 2)
    {
        struct file * f = NULL;
        struct inode * f_inode = NULL;

        // get full path
        // char buf[1000];
        // int buflen = 999;

        f = fget_raw(fd);
        if (f)
        {
            // printk full path
            // printk("Full Path: %s", dentry_path_raw(f->f_path.dentry,buf,buflen));
            f_inode = f->f_inode;
            printk("Get i_node: %lu", f_inode->i_ino);
        }
        printk("Hook S: %u\n", fd);
    }

    return real_write(regs);
}



/**
 * @brief 进行 hook
 * 
 * @return int 
 */
int prm_hook_init(void)
{
    // get syscall table position
    register_kprobe(&kp);
    sys_call_ptr = get_sys_call_table();
    printk("%s %s: System call table found at 0x%px.\n", module_name, name, sys_call_ptr);

    // hook system call
    write_protection_off(); 

    real_openat =   (void *)sys_call_ptr[__NR_openat];
    real_read =     (void *)sys_call_ptr[__NR_read];
    real_write =    (void *)sys_call_ptr[__NR_write];
    
    sys_call_ptr[__NR_openat] =     (sys_call_ptr_t)my_sys_openat;
    sys_call_ptr[__NR_read] =       (sys_call_ptr_t)my_sys_read;
    sys_call_ptr[__NR_write] =      (sys_call_ptr_t)my_sys_write;

    write_protection_on();
    printk("%s %s: System calls hook set.\n", module_name, name);

    return PRM_SUCCESS;
}

/**
 * @brief 取消 hook
 * 
 * @return int 
 */
int prm_hook_exit(void)
{
    // clear hook
    write_protection_off();

    sys_call_ptr[__NR_openat] =     (sys_call_ptr_t)real_openat;
    sys_call_ptr[__NR_read] =       (sys_call_ptr_t)real_read;
    sys_call_ptr[__NR_write] =      (sys_call_ptr_t)real_write;

    write_protection_on();
    printk("%s %s: System calls hook unset.\n", module_name, name);

    unregister_kprobe(&kp);

    return PRM_SUCCESS;
}   

// module_init(test_hook_init);
// module_exit(test_hook_exit);
