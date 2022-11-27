#include "prm_hook.h"
#include "prm_error.h"
#include "prm_netlink.h"

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

/**
 * @brief 从文件描述符获取文件的信息
 * 
 * @param fd 文件描述符
 * @param ino 对应的inode号
 * @param uid 当前的用户uid
 * @param type 文件的类型
 * @return int PRM_SUCCESS 成功, PRM_ERROR 失败
 */
int get_info_from_fd(unsigned int fd, unsigned long * ino, uid_t * uid, int *type)
{
    struct file *file_p = NULL;
    struct inode * f_inode = NULL;
    umode_t imode = 0;      // unsigned short

    // 获取 uid
    *uid = current_uid().val;   // unsigned int
    // 获取fd对应的file struct
    file_p = fget_raw(fd);
    if (!file_p)
    {
        // 获取失败
        return PRM_ERROR;
    }
    // 获取 ino
    f_inode = file_p->f_inode;
    *ino = f_inode->i_ino;

    // 获标准取文件类型
    imode = f_inode->i_mode;
    if(S_ISLNK(imode)){
        *type = FILE_LNK;
    }else if(S_ISREG(imode)){
        *type = FILE_REG;
    }else if(S_ISDIR(imode)){
        *type = FILE_DIR;
    }else if(S_ISCHR(imode)){
        *type = FILE_CHR;
    }else if(S_ISBLK(imode)){
        *type = FILE_BLK;
    }else if(S_ISFIFO(imode)){
        *type = FILE_FIFO;
    }else if(S_ISSOCK(imode)){
        *type = FILE_SOCK;
    }

    // 获取自定义文件类型
    if (fd == 0){
        *type = FILE_STDIN;
    }else if(fd == 1){
        *type = FILE_STDOUT;
    }else if(fd == 2){
        *type = FILE_STDERR;
    }

    return PRM_SUCCESS;

    // get full path
    // char buf[1000];
    // int buflen = 999;
    // printk("Full Path: %s", dentry_path_raw(f->f_path.dentry,buf,buflen));
}




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


/**
 * @brief 系统调用函数函数指针
 * 
 * 系统调用寄存器 regs 参数传递顺序
 * rdi  | rsi   | rdx   | r10
 * 1st  | 2nd   | 3rd   | 4th 
 */
typedef asmlinkage long (*sys_call_t)(struct pt_regs*);


// 原始的系统调用函数
sys_call_t real_read;
sys_call_t real_write;


/**
 * @brief 对sys_read重载
 * asmlinkage long sys_read(unsigned int fd, char __user *buf, size_t count);
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage long my_sys_read(struct pt_regs * regs)
{
    // printk("Hook succeed %lu\n", regs->di);
    return real_read(regs);
}

/**
 * @brief 对sys_write重载
 * asmlinkage long sys_write(unsigned int fd, const char __user *buf, size_t count);
 * 
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage long my_sys_write(struct pt_regs * regs)
{
    long ret = -1;

    unsigned int fd = 0;
    unsigned long ino = 0;
    uid_t uid = 0;
    int f_type = 0;
    int p_result = 0;

    fd = regs->di;
    if(get_info_from_fd(fd, &ino, &uid, &f_type) == PRM_ERROR)
    {
        // 文件标识符无法解析，直接调用原函数
        ret = real_write(regs);
    }
    else if (f_type == FILE_U)
    {
        // 文件类型无法判断，调用原函数
        ret = real_write(regs);
    }
    else if (f_type == FILE_STDIN || f_type == FILE_STDOUT || f_type == FILE_STDERR)
    {
        // IO
        ret = real_write(regs);
    }
    else if (f_type == FILE_REG)
    {
        // 常规文件
        if(ino == (unsigned long)2236977)
        {
            printk("target check\n");
            int tmp = -1;
            tmp = check_privilege(ino, uid, P_IO, &p_result);
            if(tmp == PRM_SUCCESS)
            {
                printk("Privilege check yes\n");
            }
            else
            {
                if(tmp == PRM_ERROR_SERVEROFFLINE)
                {
                    printk("Server offline\n");
                }
            }
        }
        ret = real_write(regs);
    }

    else
    {
        // printk("Hook S: %lu uid = %u type = %d\n", ino, uid, f_type);
        ret = real_write(regs);
    }
    
    return ret;
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

    // real_read =     (void *)sys_call_ptr[__NR_read];
    real_write =    (void *)sys_call_ptr[__NR_write];
    
    // sys_call_ptr[__NR_read] =       (sys_call_ptr_t)my_sys_read;
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

    // sys_call_ptr[__NR_read] =       (sys_call_ptr_t)real_read;
    sys_call_ptr[__NR_write] =      (sys_call_ptr_t)real_write;

    write_protection_on();
    printk("%s %s: System calls hook unset.\n", module_name, name);

    unregister_kprobe(&kp);

    return PRM_SUCCESS;
}   

// module_init(test_hook_init);
// module_exit(test_hook_exit);
