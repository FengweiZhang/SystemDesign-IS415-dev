#!/bin/bash

client=../user/client.exe

sudo $client -s 0 -u root -f 5

sudo ./test_reboot.out

sudo $client -s 1 -u root -f 5