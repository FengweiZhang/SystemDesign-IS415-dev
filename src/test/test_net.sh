#!/bin/bash

client=../user/client.exe

./test_net.out www.baidu.com

# 禁止pity网络权限
sudo $client -s 0 -u pity -f 6

./test_net.out www.baidu.com

# 恢复
sudo $client -s 1 -u pity -f 6

./test_net.out www.baidu.com