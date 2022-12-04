#!/bin/bash

client=../user/client.exe

# 禁止root日志访问权限
sudo $(client) -s 0 -u root -f 7
# 查看root权限（日志
sudo $(client) -g -u root -f 7

# 尝试删除日志
sudo dmesg -c

# 恢复root对日志权限
sudo $(client) -s 1 -u root -f 7

# 
sudo dmesg

sudo dmesg -c

sudo dmesg



