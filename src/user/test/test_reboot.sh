#!/bin/bash

cd ../

sudo ./client.exe -s 0 -u root -f 5

sudo ./test/test_reboot.out

sudo ./client.exe -s 1 -u root -f 5