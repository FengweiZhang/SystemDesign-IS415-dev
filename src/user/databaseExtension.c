/**
 * @file databaseExtention.c
 * @author 刘梓池 (liuzichi@sjtu.edu.cn)
 * @brief functions for Extention of database operation
 * @date 2022-11-27
 * @copyright Copyright (c) 2022
 *
 */

#include "databaseExtension.h"
#include "database.h"

/**
 * @brief check if user can access(read/write) file (only user level >= file level)
 * @param inode sqlite3 database
 * @param uid name of table to create
 * @param type name of table to create
 * @return -1: uid/inode's level do not find. 0:have permission, 1:do not have permission, 2:type donot define
**/
int user_access_file(unsigned long inode, unsigned long uid, int type){
    char uid_ch[20],ino_ch[20];

    sprintf(uid_ch,"%lu", uid);
    int level_user = db_search_right(db, "user_file", uid_ch);
    printf("search user %s level %d\n", uid_ch, level_user);

    sprintf(ino_ch,"%lu", inode);
    int level_file = db_search_right(db, "file", ino_ch);
    printf("get file %s level %d\n", ino_ch, level_file);

    if (level_file == -1 || level_user == -1){
        printf("can't find user or file's level!");
        return -1;
    }
    if (level_user >= level_file){
        printf("user %s have permission to operate file %s!", uid_ch, ino_ch);
        return 0;
    }else{
        printf("user %s don't have permission to operate file %s!", uid_ch, ino_ch);
        return 1;
    }
}