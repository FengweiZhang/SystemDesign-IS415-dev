#ifndef _PRM_HOOK_H
#define _PRM_HOOK_H


// linux 标准文件类型
#define FILE_LNK   1   // 连接文件
#define FILE_REG    2   // 常规文件
#define FILE_DIR    3   // 目录
#define FILE_CHR    4   // 字符设备
#define FILE_BLK    5   // 块设备
#define FILE_FIFO   6   // FIFO文件
#define FILE_SOCK   7   // SOCKET文件


// 自定义文件类型
#define FILE_STDIN  8   // STD IN
#define FILE_STDOUT 9   // STD OUT
#define FILE_STDERR 10  // STD ERROR

int prm_hook_init(void);

int prm_hook_exit(void);

#endif