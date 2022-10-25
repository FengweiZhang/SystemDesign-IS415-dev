#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

#define SQL_MAX_LEN 1024

static int callback(void*flag, int n_col, char**data, char**col_name){
    // if(flag==NULL) return 0;
    for(int i=0; i<n_col; ++i){
        printf("%s = %s\n", col_name[i], data[i]?data[i]:"NULL");
    }
    printf("\n");
    return 0;
}

int db_open_db(const char* db_name, sqlite3** db){
    // int res_code = -1;
    // res_code = sqlite3_open(db_name, db);
    // return res_code;
    return sqlite3_open(db_name, db);
}

int db_close_db(sqlite3* db){
    // int res_code = -1;
    // res_code = sqlite3_close(db);
    // return res_code;
    return sqlite3_close(db);
}

int db_insert_right(sqlite3* db, char* table, char* id, int level){
    //insert into <table> (id,level) values('<id>',<level>);
    char sql[SQL_MAX_LEN];
    char* err_msg;
    sprintf(sql, "insert into %s (id,level) values('%s',%d);", table, id, level);
    // printf("%s\n", sql);
    return sqlite3_exec(db, sql, callback, NULL, &err_msg);
    // return 0;
}


int db_delete_right(sqlite3* db, char* table, char* id){
    char* err_msg;
    char sql[SQL_MAX_LEN];
    sprintf(sql,"delete from %s where id = %s;", table, id);
    return sqlite3_exec(db, sql, callback, NULL, &err_msg);
    // return 0;
}


int db_update_right(sqlite3* db, char* table, char* id, int level){
    char* err_msg;
    char sql[SQL_MAX_LEN];
    sprintf(sql, "update %s set level = %d where id = %s;", table, level, id);
    return sqlite3_exec(db, sql, callback, NULL, &err_msg);
    // printf("%s\n", sql);
    // return 0;
}


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


int main(){
    sqlite3* db;
    int rc = -1;
    char *err_msg = 0;


    rc = db_open_db("database.db", &db);
    int res = db_search_right(db, "user_file", "1");
    printf("level for user_file=1 is %d\n", res);

    db_insert_right(db, "user_file", "43", 43);
    res = db_search_right(db, "user_file", "43");
    printf("level for user_file=43 is %d\n", res);
    db_delete_right(db, "user_file", "43");

    db_insert_right(db, "user_file", "123", 123);
    res = db_search_right(db, "user_file", "123");
    printf("level for user_file=123 is %d\n", res);
    db_update_right(db, "user_file", "123", 1000);
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


    

    char* sql4 = "select id,level from user_file;";
    char** result;
    int n_row;
    int n_col;
    int k = -1;

    sqlite3_get_table(db, sql4, &result, &n_row, &n_col, &err_msg);
    k = n_col;
    for(int i=0; i<n_row; ++i){
        for(int j=0; j<n_col; ++j){
            printf("%s: %s\n", result[j], result[n_col+i*n_col+j]);
        }
        printf("------------------------\n");
    }

    

    rc = db_close_db(db);
    // printf("%d\n", rc);
    return 0;
}