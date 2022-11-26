#ifndef _PRM_HOOK_H
#define _PRM_HOOK_H


#define FILE_LNK   1   // 连接文件
#define FILE_REG    2   // 常规文件
#define FILE_DIR    3   // 目录
#define FILE_CHR    4   // 字符设备
#define FILE_BLK    5   // 块设备
#define FILE_FIFO   6   // FIFO文件
#define FILE_SOCK   7   // SOCKET文件

int prm_hook_init(void);

int prm_hook_exit(void);

#endif