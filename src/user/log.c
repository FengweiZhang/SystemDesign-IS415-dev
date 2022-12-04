/**
 * @file log.c
 * @author 刘梓池 (liuzichi@sjtu.edu.cn)
 * @brief functions for log
 * @date 2022-11-29
 * @copyright Copyright (c) 2022
 *
 */

#include "log.h"
#define maxlevelnum (3)

logset logsetting;
log loging;

const static char logleveltext[4][10] = {"info", "debug", "warn", "error"};

static char *getdate(char *date);

static unsigned char getcode(char *path)
{
    unsigned char code = 255;
    if (strcmp("info", path) == 0)
        code = 1;
    else if (strcmp("warn", path) == 0)
        code = 3;
    else if (strcmp("error", path) == 0)
        code = 4;
    else if (strcmp("none", path) == 0)
        code = 0;
    else if (strcmp("debug", path) == 0)
        code = 2;
    return code;
}

static unsigned char readconfig(char *path)
{
    char value[512] = {0x0};
    char data[50] = {0x0};

    FILE *fpath = fopen(path, "r");
    if (fpath == NULL)
        return -1;
    fscanf(fpath, "path=%s\n", value);
    getdate(data);
    strcat(data, ".log");
    strcat(value, "/");
    strcat(value, data);
    if (strcmp(value, logsetting.filepath) != 0)
        memcpy(logsetting.filepath, value, strlen(value));
    memset(value, 0, sizeof(value));

    fscanf(fpath, "level=%s\n", value);
    logsetting.loglevel = getcode(value);
    fclose(fpath);
    return 0;
}
/*
 *日志设置信息
 * */
static logset *getlogset()
{
    char path[512] = {0x0};
    getcwd(path, sizeof(path));
    strcat(path, "/log.conf");
    if (access(path, F_OK) == 0)
    {
        if (readconfig(path) != 0)
        {
            logsetting.loglevel = info;
            logsetting.maxfilelen = 4096;
        }
    }
    else
    {
        logsetting.loglevel = info;
        logsetting.maxfilelen = 4096;
    }
    return &logsetting;
}

/*
 *获取日期
 * */
static char *getdate(char *date)
{
    time_t timer = time(NULL);
    strftime(date, 11, "%y-%m-%d", localtime(&timer));
    return date;
}

/*
 *获取时间
 * */
static void settime()
{
    time_t timer = time(NULL);
    strftime(loging.logtime, 20, "%y-%m-%d %h:%m:%s", localtime(&timer));
}

/*
 *不定参打印
 * */
static void printflog(char *format, va_list args)
{
    int d;
    char c, *s;
    s = va_arg(args, char *);
    fprintf(loging.logfile, format, s);
    fprintf(loging.logfile, "%s", "]\n");
}

static int initlog(unsigned char loglevel)
{
    char strdate[30] = {0x0};
    logset *logsetting;
    //获取日志配置信息
    if ((logsetting = getlogset()) == NULL)
    {
        perror("get log set fail!");
        return -1;
    }
    if ((loglevel & (logsetting->loglevel)) != loglevel)
        return -1;

    memset(&loging, 0, sizeof(log));
    //获取日志时间
    settime();
    if (strlen(logsetting->filepath) == 0)
    {
        char *path = getenv("home");
        memcpy(logsetting->filepath, path, strlen(path));

        getdate(strdate);
        strcat(strdate, ".log");
        strcat(logsetting->filepath, "/");
        strcat(logsetting->filepath, strdate);
    }
    memcpy(loging.filepath, logsetting->filepath, maxfilepath);
    // printf("logfile:%s \n", loging.filepath);
    //打开日志文件
    if (loging.logfile == NULL)
        loging.logfile = fopen(loging.filepath, "a+");
    if (loging.logfile == NULL)
    {
        perror("open log file fail!");
        return -1;
    }
    //写入日志级别，日志时间
    fprintf(loging.logfile, "[%s] [%s]:[", logleveltext[loglevel - 1], loging.logtime);
    return 0;
}

/*
 *日志写入
 * */
int logwrite(unsigned char loglevel, char *fromat, ...)
{
    va_list args;
    //初始化日志
    if (initlog(loglevel) != 0)
    {
        printf("initlog error!\n");
        return -1;
    }
    // printf("initlog success!\n");
    //打印日志信息
    va_start(args, fromat);
    printflog(fromat, args);
    va_end(args);
    //文件刷出
    fflush(loging.logfile);
    //日志关闭
    if (loging.logfile != NULL)
        fclose(loging.logfile);
    loging.logfile = NULL;
    return 0;
}