/**
 * @file log.h
 * @author 刘梓池 (liuzichi@sjtu.edu.cn)
 * @brief functions for log
 * @date 2022-11-29
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __log_h__
#define __log_h__
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdarg.h"
#include "unistd.h"

#define maxlen (2048)
#define maxfilepath (512)
#define maxfilename (50)
typedef enum
{
    error_1 = -1,
    error_2 = -2,
    error_3 = -3
} error0;

typedef enum
{
    none = 0,
    info = 1,
    debug = 2,
    warn = 3,
    error = 4,
    all = 255
} loglevel;

typedef struct log
{
    char logtime[20];
    char filepath[maxfilepath];
    FILE *logfile;
} log;

typedef struct logseting
{
    char filepath[maxfilepath];
    unsigned int maxfilelen;
    unsigned char loglevel;
} logset;

int logwrite(unsigned char loglevel, char *fromat, ...);
#endif /* __log_h__ */