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
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

// max length for each SQL command
#define SQL_MAX_LEN 1024
#define DATABASE_PATH "./database.db"

// define table name in database
#define TABLE_FILE "file"
#define TABLE_USER_FILE "user_file"
#define TABLE_PROCESS "process"
#define TABLE_USER_PROCESS "user_process"

// define other signals
#define SEPARATOR "*************************************************\n"
#define OBJECT_NOT_DEFINE -1

/**
 * @brief check if user can access(read/write) file (only user level >= file level)
 * @param db sqlite3 database
 * @param inode sqlite3 database
 * @param uid name of table to create
 * @param type name of table to create
 * @return -1: uid/inode's level do not find. 0:have permission, 1:do not have permission, 2:type donot define
 **/
int user_access_file(sqlite3 *db, unsigned long inode, unsigned long uid, int type);

#endif