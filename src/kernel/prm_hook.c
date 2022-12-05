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
#include <asm/uaccess.h>
#include <linux/fs.h>

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
    // file_p = fget_raw(fd);
    struct fd f = fdget(fd);
    file_p = f.file;
    if (!file_p)
    {
        // 获取失败
        return PRM_ERROR;
    }
    // 获取 ino
    f_inode = file_p->f_inode;
    *ino = f_inode->i_ino;

    // 获标准取文件类型
    *type = FILE_U;
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

    // 获取自定义文件类型，已经弃用
    // if (fd == 0){
    //     *type = FILE_STDIN;
    // }else if(fd == 1){
    //     *type = FILE_STDOUT;
    // }else if(fd == 2){
    //     *type = FILE_STDERR;
    // }

    // get full path
    // if (*type == FILE_DIR && *uid == 1002)
    // {
    //     char buf[1000];
    //     int buflen = 999;
    //     printk("Full Path: %s", dentry_path_raw(file_p->f_path.dentry,buf,buflen));
    // }
    return PRM_SUCCESS;

    // // get full path
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
sys_call_t real_openat;
sys_call_t real_read;
sys_call_t real_write;
sys_call_t real_reboot;
sys_call_t real_socket;
sys_call_t real_execve;


/**
 * @brief 对sys_openat进行重载
 * asmlinkage long sys_openat(int dfd, const char __user *filename, int flags, umode_t mode);
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage long my_sys_openat(struct pt_regs *regs)
{
    long ret =  real_openat(regs);

    // unsigned int fd = 0;
    unsigned long ino = 0;
    uid_t uid = 0;
    int f_type = 0;
    int p_result = 0;
    int p_type;

    uid = current_uid().val;
    if (ret < 0)
    {
        // linux 本身权限控制不通过，不处理
        p_result = CHECK_RESULT_PASS;
    }
    else
    {
        if(get_info_from_fd(ret, &ino, &uid, &f_type) == PRM_ERROR)
        {
            
            // 获取inode失败，默认通过
            p_result = CHECK_RESULT_PASS;
        }
        else
        {
            p_type = P_U;
            if (f_type == FILE_REG){
                p_type = P_REG;
            } else if (f_type == FILE_DIR){
                p_type = P_DIR;
            }

            if(p_type == P_U)
            {
                // 未定义的文件类型，调用原函数
                p_result = CHECK_RESULT_PASS;
            }
            else
            {
                int check_ret = PRM_ERROR;
                check_ret = check_privilege(ino, uid, p_type, &p_result);
                if(check_ret != PRM_SUCCESS)
                {
                    // 权限查询出错，默认通过
                    p_result = CHECK_RESULT_PASS;
                }
            }
        }
    }

    if(p_result != CHECK_RESULT_NOTPASS)
    {
        // ret =  real_openat(regs);
    }
    else
    {
        ret = -1;
        if (p_type == P_REG) printk("Block: open REG file uid=%u inode=%ld\n", uid, ino);
        if (p_type == P_DIR) printk("Block: open DIR uid=%u inode=%ld\n", uid, ino);
    }

    return ret;
    
}

/**
 * @brief 对sys_read重载
 * asmlinkage long sys_read(unsigned int fd, char __user *buf, size_t count);
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage long my_sys_read(struct pt_regs * regs)
{
    long ret = -1;

    unsigned int fd = 0;
    unsigned long ino = 0;
    uid_t uid = 0;
    int f_type = 0;
    int p_result = 0;
    int p_type;

    fd = regs->di;
    if(get_info_from_fd(fd, &ino, &uid, &f_type) == PRM_ERROR)
    {
        // 文件标识符无法解析，直接调用原函数
        p_result = CHECK_RESULT_PASS;
    }
    else if (f_type == FILE_U)
    {
        // 文件类型无法判断，调用原函数
        p_result = CHECK_RESULT_PASS;
    }
    else
    {
        // 判断权限类型
        p_type = P_U;
        if (f_type == FILE_REG){
            p_type = P_REG;         // 标准文件
        }else if (f_type == FILE_DIR){
            p_type = P_DIR;
        }
        
        

        if(p_type == P_U)
        {
            // 未定义的文件类型，调用原函数
            p_result = CHECK_RESULT_PASS;
        }
        else
        {
            int check_ret = PRM_ERROR;
            check_ret = check_privilege(ino, uid, p_type, &p_result);
            if(check_ret != PRM_SUCCESS)
            {
                // 权限查询出错，默认通过
                p_result = CHECK_RESULT_PASS;
            }
        }
    }

    // debug
    // p_result = CHECK_RESULT_PASS;

    // 判断权限检查结果，是否不允许执行
    if(p_result != CHECK_RESULT_NOTPASS)
    {
        ret = real_read(regs);
    }
    else
    {
        if (p_type == P_REG) printk("Block: read REG file uid=%u inode=%ld\n", uid, ino);
        if (p_type == P_DIR) printk("Block: read DIR uid=%u inode=%ld\n", uid, ino);
    }   
    
    return ret;
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
    int p_type;

    fd = regs->di;
    if(get_info_from_fd(fd, &ino, &uid, &f_type) == PRM_ERROR)
    {
        // 文件标识符无法解析，直接调用原函数
        p_result = CHECK_RESULT_PASS;
    }
    else if (f_type == FILE_U)
    {
        // 文件类型无法判断，调用原函数
        p_result = CHECK_RESULT_PASS;
    }
    else
    {
        // 判断权限类型
        p_type = P_U;
        if (f_type == FILE_REG){
            p_type = P_REG;         // 标准文件
        }else if (f_type == FILE_DIR){
            p_type = P_DIR;
        }

        if(p_type == P_U)
        {
            // 未定义的文件类型，调用原函数
            p_result = CHECK_RESULT_PASS;
        }
        else
        {
            int check_ret = PRM_ERROR;
            check_ret = check_privilege(ino, uid, p_type, &p_result);
            if(check_ret != PRM_SUCCESS)
            {
                // 权限查询出错，默认通过
                p_result = CHECK_RESULT_PASS;
            }
        }
    }

    // debug
    // p_result = CHECK_RESULT_PASS;

    // 判断权限检查结果，是否不允许执行
    if(p_result != CHECK_RESULT_NOTPASS)
    {
        ret = real_write(regs);
    }
    else
    {
        if (p_type == P_REG) printk("Block: write REG file uid=%u inode=%ld\n", uid, ino);
        if (p_type == P_DIR) printk("Block: write DIR uid=%u inode=%ld\n", uid, ino);
    }
    
    return ret;
}

/**
 * @brief 对sys_reboot重载
 * asmlinkage long sys_reboot(int magic1, int magic2, unsigned int cmd, void __user *arg);
 * 只有root用户才可以使用reboot
 * 因此只对root用户的reboot的重启权限进行限制
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage long my_sys_reboot(struct pt_regs * regs)
{
    long ret = -1;
    uid_t uid;
    int p_result = 0;

    uid = current_uid().val;
    // printk("reboot!!!!!%u\n", uid);

    if (uid == 0)
    {
        // 是root用户
        int check_ret = PRM_ERROR;
        // inode o for error
        check_ret = check_privilege(0, uid, P_REBOOT, &p_result);
        if(check_ret != PRM_SUCCESS)
        {
            // 权限查询出错，默认通过
            p_result = CHECK_RESULT_PASS;
        }
        // 若成功查询，结果已经放入了p_result中

        // debug 先设置为都通过
        // p_result = CHECK_RESULT_PASS;
    }
    else
    {
        // 普通用户无权调用reboot，由linux系统自行判断
        p_result = CHECK_RESULT_PASS;
    }

    if (p_result != CHECK_RESULT_NOTPASS)
    {
        ret = real_reboot(regs);
    }
    else
    {
        printk("Block: reboot %u\n", uid);
    }
    return ret;
}

/**
 * @brief 对sys_socket进行重载
 * asmlinkage long sys_socket(int, int, int);
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage long my_sys_socket(struct pt_regs *regs)
{
    long ret = -1;
    uid_t uid;
    int p_result = 0;
    int check_ret = PRM_ERROR;

    uid = current_uid().val;
    // printk("socket create %u\n", uid);
    check_ret = check_privilege(0, uid, P_NET, &p_result);
    if(check_ret != PRM_SUCCESS)
    {
        // 权限查询出错，默认通过
        p_result = CHECK_RESULT_PASS;
    }

    // debug 都通过
    // p_result = CHECK_RESULT_PASS;

    if(p_result != CHECK_RESULT_NOTPASS)
    {
        ret = real_socket(regs);
    }
    else
    {
        printk("Block: net %u\n", uid);
    }
    return ret;
}

/**
 * @brief 对sys_execve进行hook
 * asmlinkage long sys_execve(const char __user *filename,
		const char __user *const __user *argv,
		const char __user *const __user *envp);
 * 
 * @param regs 
 * @return asmlinkage 
 */
asmlinkage long my_sys_execve(struct pt_regs *regs)
{
    long ret = -1;
    uid_t uid;
    int p_result = 0;
    int check_ret = PRM_ERROR;
    int memcpy_ret = -1;

    // linux 下路经最长为4096byte
    char filename[4096];
    char dmesg_name[64] = "/usr/bin/dmesg";

    uid = current_uid().val;
    // printk("execve: %u: \n", uid);
    // kernel 不能直接操作user space的内存，因此需要现复制
    // 如果成功返回0；如果失败，返回有多少个Bytes未完成copy
    memcpy_ret = copy_from_user(filename, (char *)(regs->di), 4096);
    if(memcpy_ret != 0)
    {
        // 从用户态复制数据出错
        p_result = CHECK_RESULT_PASS;
    }
    else
    {
        if (strcmp(dmesg_name, filename) == 0)
        {
            // 是 demsg 命令
            // printk("execv: %s uid=%u\n", filename, uid);
            check_ret = check_privilege(0, uid, P_DEMESG, &p_result);
            if(check_ret != PRM_SUCCESS)
            {
                // 权限查询出错，默认通过
                p_result = CHECK_RESULT_PASS;
            }
        }
        else
        {
            p_result = CHECK_RESULT_PASS;    
        }
    }
    // printk("%s\n", filename);

    // debug
    // p_result = CHECK_RESULT_PASS;

    if(p_result != CHECK_RESULT_NOTPASS)
    {
        ret = real_execve(regs);
    }
    else
    {
        printk("Block: dmesg %u\n", uid);
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

    // 保存原系统调用函数
    real_openat =   (void *)sys_call_ptr[__NR_openat];
    real_read =     (void *)sys_call_ptr[__NR_read];
    real_write =    (void *)sys_call_ptr[__NR_write];
    real_reboot =   (void *)sys_call_ptr[__NR_reboot];
    real_socket =   (void *)sys_call_ptr[__NR_socket];
    real_execve =   (void *)sys_call_ptr[__NR_execve];

    // 修改系统调用表
    sys_call_ptr[__NR_openat] =     (sys_call_ptr_t)my_sys_openat;
    sys_call_ptr[__NR_read] =       (sys_call_ptr_t)my_sys_read;
    sys_call_ptr[__NR_write] =      (sys_call_ptr_t)my_sys_write;
    sys_call_ptr[__NR_reboot] =     (sys_call_ptr_t)my_sys_reboot;
    sys_call_ptr[__NR_socket] =     (sys_call_ptr_t)my_sys_socket;
    sys_call_ptr[__NR_execve] =     (sys_call_ptr_t)my_sys_execve;

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

    // 恢复系统调用表
    sys_call_ptr[__NR_openat] =     (sys_call_ptr_t)real_openat;
    sys_call_ptr[__NR_read] =       (sys_call_ptr_t)real_read;
    sys_call_ptr[__NR_write] =      (sys_call_ptr_t)real_write;
    sys_call_ptr[__NR_reboot] =     (sys_call_ptr_t)real_reboot;
    sys_call_ptr[__NR_socket] =     (sys_call_ptr_t)real_socket;
    sys_call_ptr[__NR_execve] =     (sys_call_ptr_t)real_execve;

    write_protection_on();
    printk("%s %s: System calls hook unset.\n", module_name, name);

    unregister_kprobe(&kp);

    return PRM_SUCCESS;
}   

// module_init(test_hook_init);
// module_exit(test_hook_exit);
