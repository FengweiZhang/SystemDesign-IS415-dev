#!/bin/bash

cd ..

echo "\n禁止pity的网络权限"
sudo ./client.exe -s 0 -u pity -f 6

echo "\npity尝试访问网络"
su pity -c ./test/test_net.out

echo "\n恢复pity的网路权限"
sudo ./client.exe -s 1 -u pity -f 6

echo "\npity尝试访问网络"
su pity -c ./test/test_net.out