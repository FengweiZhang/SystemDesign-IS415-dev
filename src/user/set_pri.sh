#!/bin/bash

# 设置用户的权限
sudo ./client.exe -s 1 -u pity1
sudo ./client.exe -s 2 -u pity2
sudo ./client.exe -s 3 -u pity3
sudo ./client.exe -s 4 -u pity4


# 设置文件的权限
sudo ./client.exe -s 1 -f /home/ubuntu/test/test1
sudo ./client.exe -s 2 -f /home/ubuntu/test/test2
sudo ./client.exe -s 3 -f /home/ubuntu/test/test3
sudo ./client.exe -s 4 -f /home/ubuntu/test/test4


# 查看用户权限
sudo ./client.exe -g -u pity1
sudo ./client.exe -g -u pity2
sudo ./client.exe -g -u pity3
sudo ./client.exe -g -u pity4

# 查看文件权限
sudo ./client.exe -g -f /home/ubuntu/test/test1
sudo ./client.exe -g -f /home/ubuntu/test/test2
sudo ./client.exe -g -f /home/ubuntu/test/test3
sudo ./client.exe -g -f /home/ubuntu/test/test4
