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
#endif

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

// max length for each SQL command
#define SQL_MAX_LEN 1024

static int callback(void*flag, int n_col, char**data, char**col_name){
    // if(flag==NULL) return 0;
    for(int i=0; i<n_col; ++i){
        printf("%s = %s\n", col_name[i], data[i]?data[i]:"NULL");
    }
    printf("\n");
    return 0;
}
/**
    @param db_name name of database
    @param db database
**/
int db_open_db(const char* db_name, sqlite3** db){
    return sqlite3_open(db_name, db);
}


int db_close_db(sqlite3* db){
    return sqlite3_close(db);
}

/**
    @brief insert a row into database's table, assign a right level of an user/object
    @param db database
    @param table name of table in database
    @param id id of an user or an object
    @param level the right level of corresponding user/object
**/
int db_insert_right(sqlite3* db, char* table, char* id, int level){
    //insert into <table> (id,level) values('<id>',<level>);
    char sql[SQL_MAX_LEN];
    char* err_msg;
    sprintf(sql, "insert into %s (id,level) values('%s',%d);", table, id, level);
    return sqlite3_exec(db, sql, callback, NULL, &err_msg);
}

/**
    @brief delete the level of an user/object
    @param db database
    @param table name of table in database
    @param id id of an user or an object
**/
int db_delete_right(sqlite3* db, char* table, char* id){
    char* err_msg;
    char sql[SQL_MAX_LEN];
    sprintf(sql,"delete from %s where id = %s;", table, id);
    return sqlite3_exec(db, sql, callback, NULL, &err_msg);
}

/**
    @brief update the right level of an user/object
    @param db database
    @param table name of table in database
    @param id id of an user or an object
    @param level the right level of corresponding user/object
**/
int db_update_right(sqlite3* db, char* table, char* id, int level){
    char* err_msg;
    char sql[SQL_MAX_LEN];
    sprintf(sql, "update %s set level = %d where id = %s;", table, level, id);
    return sqlite3_exec(db, sql, callback, NULL, &err_msg);
}

/**
    @brief given id, search the right of corresponding user/object
    @param db database
    @param table name of table in database
    @param id id of an user or an object
    @return the level, which is int type
**/
int db_search_right(sqlite3* db, char* table, char* id){
    // select level from <table> where id = <id>;
    char sql[SQL_MAX_LEN];
    sprintf(sql,"select level from %s where id = %s;", table, id);
    char** result;
    char* err_msg;
    int n_row;
    int n_col;

    sqlite3_get_table(db, sql, &result, &n_row, &n_col, &err_msg);
    return atoi(result[n_col]);

}