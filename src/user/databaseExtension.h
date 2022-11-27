/**
 * @file databaseExtention.H
 * @author 刘梓池 (liuzichi@sjtu.edu.cn)
 * @brief functions for Extention of database operation
 * @date 2022-11-27
 * @copyright Copyright (c) 2022
 *
 */


#ifndef __DATABASEEXTENSION__
#define __DATABASEEXTENSION__

#include "databaseExtension.h"
#include "database.h"

/**
 * @brief check if user can access(read/write) file (only user level >= file level)
 * @param inode sqlite3 database
 * @param uid name of table to create
 * @param type name of table to create
 * @return -1: uid/inode's level do not find. 0:have permission, 1:do not have permission, 2:type donot define
**/
int user_access_file(unsigned long inode, unsigned long uid, int type);

#endif