#!/bin/bash

cd ../

echo "\n禁止root用户reboot权限"
sudo ./client.exe -s 0 -u root -f 5

echo "\nroot用户尝试重启计算机"
sudo ./test/test_reboot.out

echo "\n启动root用户reboot权限"
sudo ./client.exe -s 1 -u root -f 5
