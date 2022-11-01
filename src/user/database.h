/**
 * @file database.h
 * @author 钟睿哲 (zhongruizhe271828@foxmail.com, zerzerzerz271828@sjtu.edu.cn)
 * @brief functions for database operation
 * @date 2022-10-25
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __DATABASE__
#define __DATABASE__

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


static int callback(void*flag, int n_col, char**data, char**col_name);


/**
 * @brief create a new table in database
 * @param db sqlite3 database
 * @param table name of table to create
**/
int db_create_table(sqlite3* db, char* table);


/**
    @param db_name name of database
    @param db database
**/
int db_open_db(const char* db_name, sqlite3** db);
int db_close_db(sqlite3* db);


/**
    @brief insert a row into database's table, assign a right level of an user/object
    @paragraph if the id is existed in table, the right will not be updated
    @param db database
    @param table name of table in database
    @param id id of an user or an object
    @param level the right level of corresponding user/object
**/
int db_insert_right(sqlite3* db, char* table, char* id, int level);


/**
    @brief delete the level of an user/object
    @param db database
    @param table name of table in database
    @param id id of an user or an object
**/
int db_delete_right(sqlite3* db, char* table, char* id);


/**
    @brief update the right level of an user/object
    @param db database
    @param table name of table in database
    @param id id of an user or an object
    @param level the right level of corresponding user/object
**/
int db_update_right(sqlite3* db, char* table, char* id, int level);


/**
    @brief given id, search the right of corresponding user/object
    @param db database
    @param table name of table in database
    @param id id of an user or an object
    @return the level, which is int type
**/
int db_search_right(sqlite3* db, char* table, char* id);


/**
 * @brief set the level user/object. If exists, update level, else insert new level
 * **/
int db_set_right(sqlite3* db, char* table, char* id, int level);


/**
    @brief show a table in database
    @param db sqlite3 database
    @param table name of table
**/
int db_show_table(sqlite3* db, char* table);

#endif
