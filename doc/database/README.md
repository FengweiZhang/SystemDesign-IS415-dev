- `sqlite3_get_table`的查询结果以这样的格式进行存储
  - `[col_name1, col_name2, ..., col_nameN, val1, val2, ... valN, ..., val1, val2, ... valN,]`

## 环境配置
- 若跑不通，运行
  - `sudo apt-get update`
  - `sudo apt-get upgrade`
  - `sudo apt-get install libsqlite3-dev`

## 数据库简介
- `database\database.db`存放着各类数据，目前其中有如下table，每个table都有两个字段，分别是`id(char(50))`, `level(int)`
  - `user_file`,存储用户在文件类型中的权限等级
  - `file`, 每个文件的权限等级
  - `user_process`,存储用户在进程类型中的权限等级
  - `process`, 每个进程的权限等级