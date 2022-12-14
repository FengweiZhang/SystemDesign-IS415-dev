/**
 * @file test.c
 * @author 钟睿哲 (zhongruizhe271828@foxmail.com, zerzerzerz271828@sjtu.edu.cn)
 * @brief functions for database operation
 * @date 2022-10-25
 * @copyright Copyright (c) 2022
 *
 */

#include "database.h"

int main(){
    sqlite3* db;
    int rc = -1;
    char *err_msg = 0;

    rc = db_open_db(DATABASE_PATH, &db);
    db_show_table(db, TABLE_USER_FILE);
    char user_id[1024];
    printf("Please enter user id:");
    scanf("%s", user_id);
    int res = db_search_right(db, TABLE_USER_FILE, user_id);
    printf("Level for user %s in table %s is %d\n", user_id, TABLE_USER_FILE, res);
    return 0;

    res = db_search_right(db, TABLE_USER_FILE, "1");
    printf("level for user_file=1 is %d\n", res);

    db_insert_right(db, TABLE_USER_FILE, "43", 43);
    res = db_search_right(db, TABLE_USER_FILE, "43");
    printf("level for user_file=43 is %d\n", res);
    db_delete_right(db, TABLE_USER_FILE, "43");

    db_insert_right(db, TABLE_USER_FILE, "123", 123);
    res = db_search_right(db, TABLE_USER_FILE, "123");
    printf("level for user_file=123 is %d\n", res);
    db_update_right(db, TABLE_USER_FILE, "123", 1000);

    
    db_create_table(db, "test");
    db_insert_right(db, "test", "zrz", 1);
    db_insert_right(db, "test", "lzz", 2);
    db_insert_right(db, "test", "zfw", 3);

    

    db_set_right(db, TABLE_USER_FILE, "123456", 16);
    db_show_table(db, TABLE_USER_FILE);
    db_set_right(db, TABLE_USER_FILE, "123456", 456456);
    db_show_table(db, TABLE_USER_FILE);

    db_show_table(db, TABLE_FILE);
    db_show_table(db, TABLE_USER_FILE);
    db_show_table(db, TABLE_PROCESS);
    db_show_table(db, TABLE_USER_PROCESS);

    rc = db_close_db(db);
    return 0;

    // char* tables[] = {
    //     "file",
    //     TABLE_USER_FILE,
    //     "process",
    //     "user_process",
    //     "test"
    // };
    // for(int i=0; i<5; ++i){
    //     db_show_table(db, tables[i]);
    // }

    // user_file
    // file
    // user_process
    // process

    // char* sql = "create table process (id char(50) primary key not null, level int not null);";
    // char* sql2 = "insert into process (id,level) values('0',0);";
    // char* sql3 = "insert into process (id,level) values('1',1);";
    // sqlite3_exec(db, sql, callback, NULL, &err_msg);
    // sqlite3_exec(db, sql2, callback, NULL, &err_msg);
    // sqlite3_exec(db, sql3, callback, NULL, &err_msg);




    // printf("%d\n", rc);

    // char* sql = "CREATE TABLE COMPANY("  \
    //     "ID INT PRIMARY KEY     NOT NULL," \
    //     "NAME           TEXT    NOT NULL," \
    //     "AGE            INT     NOT NULL," \
    //     "ADDRESS        CHAR(50)," \
    //     "SALARY         REAL );";
    // sqlite3_exec(db, sql, callback, NULL, &err_msg);

    // char* sql2 = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
    //     "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
    //     "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
    //     "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
    //     "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
    //     "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
    //     "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
    //     "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";
    // sqlite3_exec(db, sql2, callback, NULL, &err_msg);

    // char* sql3 = "select * from company;";
    // sqlite3_exec(db, sql3, callback, 0, &err_msg);


    // char* sql4 = "select id,level from user_file;";
    // char** result;
    // int n_row;
    // int n_col;
    // int k = -1;

    // sqlite3_get_table(db, sql4, &result, &n_row, &n_col, &err_msg);
    // k = n_col;
    // for(int i=0; i<n_row; ++i){
    //     for(int j=0; j<n_col; ++j){
    //         printf("%s: %s\n", result[j], result[n_col+i*n_col+j]);
    //     }
    //     printf("------------------------\n");
    // }
}