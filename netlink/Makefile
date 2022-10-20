obj-m:=my_netlink.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
