#!/bin/bash

cd ..

echo "\n禁止root日志访问权限"
sudo ./client.exe -s 0 -u root -f 7

echo "\nroot用户尝试删除日志"
sudo dmesg -C

echo "\n恢复root对日志权限"
sudo ./client.exe -s 1 -u root -f 7

echo "\nroot用户尝试查看日志"
sudo dmesg | tail -n 20

echo "\nroot用户尝试删除日志"
sudo dmesg -C

echo "\nroot用户检查日志"
sudo dmesg



